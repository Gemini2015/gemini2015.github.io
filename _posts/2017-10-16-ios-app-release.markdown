---
layout: post
title: iOS游戏上架参考
categories: ios
description: 针对iOS游戏上架AppStore需要处理的问题的汇总
codelang: shell
---

## 前言

相比于Android应用商店（Google Play除外），iOS应用商店的管理更加规范，仔细，审核也更加严格。以下列出了iOS游戏上架AppStore需要注意的一些地方（重点针对基于Unity开发的iOS游戏）。


## IPv6支持
上架AppStore的应用需支持IPv6环境。以下为从网上收集到的解决方案，已经过测试可用。
编写iOS Native Plugin`ipv6.mm`
```
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>

#define NSSTRING_TO_STR_OR_EMPTY(t) \
	(((t) == nil) ? NULL : strdup([(t) UTF8String]))

extern "C"
{
    const char* _IOSNativeExt_CheckIPAddress(const char* host);
}

const char* _IOSNativeExt_CheckIPAddress(const char* host)
{
    if( nil == host )
        return NULL;
    const char *newChar = "No";
    struct addrinfo* res0;
    struct addrinfo hints;
    struct addrinfo* res;
    int n, s;

    memset(&hints, 0, sizeof(hints));

    hints.ai_flags = AI_DEFAULT;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((n=getaddrinfo(host, "http", &hints, &res0))!=0)
    {
        printf("getaddrinfo error: %s\n",gai_strerror(n));
        return NULL;
    }

    struct sockaddr_in6* addr6;
    struct sockaddr_in* addr;
    NSString * NewStr = NULL;
    char ipbuf[32];
    s = -1;
    for(res = res0; res; res = res->ai_next)
    {
        if (res->ai_family == AF_INET6)
        {
            addr6 =( struct sockaddr_in6*)res->ai_addr;
            newChar = inet_ntop(AF_INET6, &addr6->sin6_addr, ipbuf, sizeof(ipbuf));
            NSString * TempA = [[NSString alloc] initWithCString:(const char*)newChar
                                                        encoding:NSASCIIStringEncoding];
            NSString * TempB = [NSString stringWithUTF8String:"&ipv6"];

            NewStr = [TempA stringByAppendingString: TempB];
            printf("%s\n", newChar);
        }
        else
        {
            addr =( struct sockaddr_in*)res->ai_addr;
            newChar = inet_ntop(AF_INET, &addr->sin_addr, ipbuf, sizeof(ipbuf));
            NSString * TempA = [[NSString alloc] initWithCString:(const char*)newChar
                                                        encoding:NSASCIIStringEncoding];
            NSString * TempB = [NSString stringWithUTF8String:"&ipv4"];

            NewStr = [TempA stringByAppendingString: TempB];
            printf("%s\n", newChar);
        }
        break;
    }

    freeaddrinfo(res0);

    NSString * mIPaddr = NewStr;
    return NSSTRING_TO_STR_OR_EMPTY(mIPaddr);
}

```
C#脚本里面，进行如下判断
```
if(isIPv6)
{
	var socket = new Socket(AddressFamily.InterNetworkV6, SocketType.Stream, ProtocolType.Tcp);
	socket.Bind(new IPEndPoint(IPAddress.IPv6Any, 0));
}
else
{
	var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
	socket.Bind(new IPEndPoint(IPAddress.Any, 0));
}
```


## Build检查

*	BundleID
*   IL2CPP：上架AppStore必须使用IL2CPP。iOS 11之后，使用Mono后端，无法真机调试。
*	Version & Build：Version 和 Build 不能重复。即已经上传了一个Version为`0.0.1`，Build为`1`的ipa包到iTunes Connect之后，若要再重新上传一个Version`0.0.1`的包，须将Build改为`2`或其他值。（Android同理）
*	权限声明：
	*	`NSPhotoLibraryUsageDescription`：访问相册
	*	`NSMicrophoneUsageDescription`：访问麦克风
	*	`NSCameraUsageDescription`：访问相机
	*	`NSLocationWhenInUseUsageDescription`：访问位置信息
*	取消 Development Build

## Xcode设置
因为**[XUPorter](https://github.com/onevcat/XUPorter)**已经不更新了，建议在Unity里面使用Unity官方提供的**[XcodeAPI](https://bitbucket.org/Unity-Technologies/xcodeapi)**来设置Xcode工程。
以下是一个简单的设置Demo
```
public static void XcodeSetup(string pathToBuiltProject)
{
	string projPath = PBXProject.GetPBXProjectPath(pathToBuiltProject);
	string targetName = PBXProject.GetUnityTargetName();

	PBXProject proj = new PBXProject();
	proj.ReadFromString(File.ReadAllText(projPath));

	string targetGuid = proj.TargetGuidByName(targetName);

	// libz.tbd
	proj.AddFrameworkToProject(targetGuid, "libz.tbd", false);

	// SystemConfiguration.framework
	proj.AddFrameworkToProject(targetGuid, "SystemConfiguration.framework", false);

	// 普通文件
	var shareIcon = Path.Combine(Application.dataPath, "../shareicon.png");
    var shareIconFile = proj.AddFile(shareIcon, "SDK/shareicon.png", PBXSourceTree.Source);
    proj.AddFileToBuild(targetGuid, shareIconFile);

    // 编译选项
    proj.AddBuildProperty(targetGuid, "OTHER_LDFLAGS", "-ObjC");
    proj.SetBuildProperty(targetGuid, "ENABLE_BITCODE", "NO");

    // 证书签名
    // cert: "iPhone Developer: name (id)"
    // provision: "provisioning_profile_name"(非ID)
    proj.SetBuildProperty(targetGuid, "PROVISIONING_PROFILE_SPECIFIER", provision);
	proj.SetBuildProperty(targetGuid, "CODE_SIGN_IDENTITY", cert);
	proj.SetTeamId(targetGuid, teamID);

	proj.WriteToFile(projPath);



	// Info.plist
	var plistPath = Path.Combine(pathToBuiltProject, "Info.plist");
    var plist = new PlistDocument();
    plist.ReadFromFile(plistPath);

    // CFBundleURLTypes
    var array = plist.root.CreateArray("CFBundleURLTypes");

    var urlDict = array.AddDict();
    urlDict.SetString("CFBundleTypeRole", "Editor");
    urlDict.SetString("CFBundleURLName", "weixin");
    var urlInnerArray = urlDict.CreateArray("CFBundleURLSchemes");
	urlInnerArray.AddString("wx_app_id");

    // CFBundleURLTypes
    array = plist.root.CreateArray("LSApplicationQueriesSchemes");
    array.AddString("mw");

    plist.root.SetString("NSPhotoLibraryUsageDescription", "此 App 需要您的同意才能读取媒体资料库");

    plist.WriteToFile(plistPath);



    // Capability
    var postfix = "BundleIDPostfix"; // com.abc.xyz -> xyz
	ProjectCapabilityManager capManager = new ProjectCapabilityManager(projPath, postfix + ".entitlements", targetName);

    capManager.AddInAppPurchase();

    capManager.WriteToFile();
}

```


## 生成ipa
旧版`xcrun`命令已经失效，打出来的包签名验证不过。建议使用`xcodebuild`命令完成`build`，`archive`操作。
Xcode设置好之后，可以使用下面的命令，生成商店ipa包。

1.	Clean：`xcodebuild  -verbose  -project /path/to/UnityBuildOutput/Unity-iPhone.xcodeproj -target  Unity-iPhone  -configuration  Release  clean`
2.	Build：`xcodebuild  archive  -project /path/to/UnityBuildOutput/Unity-iPhone.xcodeproj -scheme Unity-iPhone  -archivePath /path/to/xcodeProjectArchive/bundleIDPostfix.xcarchive -configuration  Release  -jobs 8`
3.	Archive：`xcodebuild  -verbose -exportArchive  -archivePath /path/to/xcodeProjectArchive/bundleIDPostfix.xcarchive -exportOptionsPlist /path/to/exportOptionFiles/exportAppStore.plist -exportPath /path/to/exportIPA`

*exportAppStore.plist*的文件内容如下
```
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>compileBitcode</key>
    <false/>
    <key>uploadBitcode</key>
    <false/>
    <key>uploadSymbols</key>
    <false/>
    <key>method</key>
    <string>app-store</string>
</dict>
</plist>
```


## 提交ipa
在**[iTunes Connect](https://itunesconnect.apple.com)**上完成基本设置。  
在Mac机器上，通过Xcode->Open Developer Tool启动`Application Loader`，选择**交付您的应用**，选取对应的ipa包，然后一直点下一步就可以了。  
上传完成之后，需要经过一段时间的处理，处理完成之后就可以选择该构建版本进行审核发布。  
关于iTunes Connect设置的具体细节，可以自行搜索。


## 关于iOS内购

### 准备工作

*	在[iTunes Connect]上填写**协议、税务和银行业务**信息，未填写会导致`invalidProductIdentifiers`错误。
*	添加产品：在iTunes Connect上添加产品，产品ID可以自定义。
*	添加沙盒测试账号：需用真实存在的，且未作为AppleID的邮箱，后续需要邮件验证。
*	在[开发者后台](https://developer.apple.com)为App ID添加`In-App Purchase`服务，**Development**和**Distribution**都要开通。
*	更新**Provisioning Profiles**

**[iTunes Connect内购设置参考](http://www.jianshu.com/p/86ac7d3b593a)**

### 开发

内购代码示例可自行搜索。