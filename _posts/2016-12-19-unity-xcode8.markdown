---
layout: post
title: Unity3D导出Xcode 8编译
categories: Game
description: 在Xcode 8环境下，通过脚本编译打包Unity3D项目
codelang: csharp
---

## 前言
本文基于现有Unity3D项目已经使用`XUPorter`和`Bash`脚本在Xcode 7环境下可以正常编译打包。
Unity3D版本`5.3.5p8`
Xcode版本`8.2`


## Xcode 8 主要变化

＊   `Target`增加`Automatically manage signing`选项，勾选可以通过联网自动设置`Provision`、`TeamID`等。若不勾选，则需手动分别设置`Debug`和`Release`的`Provision`。
＊   在Xcode 7环境下，使用`xcodebuild`编译时，只需要指定`CODE_SIGN_IDENTITY`参数。在Xcode 8环境中，还需要指定`PROVISIONING_PROFILE_SPECIFIER`和`DEVELOPMENT_TEAM`。
＊   对证书的权限控制有所修改(猜的)。


## 主要任务

＊   Unity3D 生成的Xcode工程，`Target`没有`Automatically manage signing`选项，如果没有该选项，在使用`xcodebuild`进行编译时会提示如下信息
    ```
    DemoProject requires a provisioning profile. Select a provisioning profile for the "Debug" build configuration in the project editor.
    Code signing is required for product type 'Application' in SDK 'iOS 10.2'
    ```
    因此，需要在Unity3D生成的Xcode工程里为`Target`添加`Automatically manage signing`选项。

＊   在执行`xcodebuild`命令时，增加`PROVISIONING_PROFILE_SPECIFIER`和`DEVELOPMENT_TEAM`参数。


## 实现

### Signing选项

`Target`的`Automatically manage signing`选项是保存在`ProjectName.xcodeproj/project.pbxproj`文件里。例如

```
/* Begin PBXDictionary section */
29B97313FDCFA39411CA2CEA /* Project object */ = {
    isa = PBXProject;
    attributes = {
        TargetAttributes = {
            5623C57217FDCB0800090B9E /* Unity-iPhone Tests */ = {
                ProvisioningStyle = Manual;
            };
            1D6058900D05DD3D006BFB54 /* Unity-iPhone */ = {
                ProvisioningStyle = Manual;
            };
        };
    };
    buildConfigurationList = C01FCF4E08A954540054247B /* Release */;
    compatibilityVersion = "Xcode 3.2";
    developmentRegion = English;
    hasScannedForEncodings = 1;
    knownRegions = (
        English,
        Japanese,
        French,
        German,
        en,
    );
    mainGroup = 29B97314FDCFA39411CA2CEA /* CustomTemplate */;
    projectDirPath = "";
    projectRoot = "";
    targets = (
        1D6058900D05DD3D006BFB54 /* Unity-iPhone */,
        5623C57217FDCB0800090B9E /* Unity-iPhone Tests */,
    );
};
/* End PBXDictionary section */
```

`ProvisioningStyle`这个字段，对应的就是`1D6058900D05DD3D006BFB54`这个`Target`的`Automatically manage signing`选项，它有两个候选值`Automatic`和`Manual`。
通过Xcode 8建的工程，默认值是`Automatic`，通过Unity3D生成的Xcode工程，没有这个字段。

通过Unity3D生成的Xcode工程，Project object是下面这个样子的。

```
/* Begin PBXDictionary section */
29B97313FDCFA39411CA2CEA /* Project object */ = {
    isa = PBXProject;
    attributes = {
        TargetAttributes = {
            5623C57217FDCB0800090B9E /* Unity-iPhone Tests */ = {
                TestTargetID = 1D6058900D05DD3D006BFB54 /* Unity-iPhone */;
            };
        };
    };
    buildConfigurationList = C01FCF4E08A954540054247B /* Release */;
    compatibilityVersion = "Xcode 3.2";
    developmentRegion = English;
    hasScannedForEncodings = 1;
    knownRegions = (
        English,
        Japanese,
        French,
        German,
        en,
    );
    mainGroup = 29B97314FDCFA39411CA2CEA /* CustomTemplate */;
    projectDirPath = "";
    projectRoot = "";
    targets = (
        1D6058900D05DD3D006BFB54 /* Unity-iPhone */,
        5623C57217FDCB0800090B9E /* Unity-iPhone Tests */,
    );
};
/* End PBXDictionary section */
```

我们需要在`TargetAttributes`这个属性中，根据`targets`枚举每一个`Target`，为每一个`Target`增加`ProvisioningStyle`设置。
这个任务可以通过`XUPorter`来实现。代码如下。

PBXProject做如下修改

```
Index: PBX Editor/PBXProject.cs
===================================================================
--- PBX Editor/PBXProject.cs    (old)
+++ PBX Editor/PBXProject.cs    (new)
@@ -8,6 +8,8 @@
    {
        protected string MAINGROUP_KEY = "mainGroup";
        protected string KNOWN_REGIONS_KEY = "knownRegions";
+       protected string TARGETS_KEY = "targets";
+       protected string ATTRIBUTES_KEY = "attributes";

        protected bool _clearedLoc = false;

@@ -29,6 +31,22 @@
            }
        }

+       public PBXList targets
+       {
+           get
+           {
+               return (PBXList)_data [TARGETS_KEY];
+           }
+       }
+
+       public PBXDictionary attributes
+       {
+           get
+           {
+               return (PBXDictionary)_data [ATTRIBUTES_KEY];
+           }
+       }
+

```


XCodePostProcess做如下修改

```
Index: XCodePostProcess.cs
===================================================================
--- XCodePostProcess.cs (old)
+++ XCodePostProcess.cs (new)
@@ -43,7 +43,30 @@
        project.overwriteBuildSetting("GCC_ENABLE_OBJC_EXCEPTIONS", "YES", "Release");
        project.overwriteBuildSetting("GCC_ENABLE_OBJC_EXCEPTIONS", "YES", "Debug");

+       var pbxproj = project.project;

+       var attrs = pbxproj.attributes;
+       var targetAttrs = (PBXDictionary)attrs["TargetAttributes"];
+       PBXDictionary targetSetting = new PBXDictionary ();
+       targetSetting["ProvisioningStyle"] = "Manual";
+       targetSetting["DevelopmentTeam"] = "YourTeamID(Optional)";
+
+       var targets = pbxproj.targets;
+       foreach (var t in targets)
+       {
+           var targetID = (string)t;
+           if (targetAttrs.ContainsKey (targetID))
+           {
+               var TargetAttr = (PBXDictionary)targetAttrs [targetID];
+               TargetAttr.Append (targetSetting);
+           }
+           else
+           {
+               targetAttrs [targetID] = targetSetting;
+           }
+
+       }
+

```

### xcodebuild参数

为`xcodebuild`增加如下参数:

*   `PROVISIONING_PROFILE_SPECIFIER`设置为`Provision`的名称，区分`Debug`与`Release`。
*   `DEVELOPMENT_TEAM`设置为证书所属的`TeamID`。


## Tips

*   之前因为权限问题，将钥匙串中的开发证书都设置成了***始终信任***，但是在Xcode 8中，会导致签名失败（即使你已经正确设置了`Provision`），设置成***系统默认***可以解决这个问题。
*   Xcode 7的`svn`版本为`1.7.22`，Xcode 8的`svn`版本为`1.9.4`，新版svn会升级文件格式，故敬慎升级。