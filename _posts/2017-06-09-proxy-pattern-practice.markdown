---
layout: post
title: 代理模式在异步编程中的实践
categories: Game
description: 通过代理模式来解决异步编程中存在的一些问题
codelang: csharp
---


## 一个简单的例子

假设有这样一个需求，你从服务器收到一批怪物的数据，需要根据怪物的数据在地图上创建出具体的怪物，然后服务器还有可能随时发送消息来通知你更新或者删除怪物。
可以采用下面的方式实现。

```
// 挂在怪物资源上
class Monster: MonoBehaviour
{
    private int id = 0;

	private MonsterData monsterData = null;

	public void SetMonsterData(int id, MonsterData data)
	{
        this.id = id;
		monsterData = data;

		// ...
		// 根据数据更新怪物
	}
}

class MonsterManager
{
    private Dictionary<int, Monster> monsters = new Dictionary<int, Monster>();

    public void CreateMonster(int id, MonsterData data)
    {
        if(monsters.ContainsKey(id))
            return;

        var go = ResourceManager.Load(data.prefabPath);
        var monster = go.GetComponent<Monster>();
        monster.SetMonsterData(data);
        monsters[id] = monster;
    }

    public void UpdateMonster(int id, MonsterData data)
    {
        if(!monsters.ContainsKey(id))
            return;

        var monster = monsters[id];
        monster.SetMonsterData(data);
    }

    public void RemoveMonster(int id)
    {
        if(monsters.ContainsKey(id))
        {
			var monster = monsters[id];
			GameObject.Destroy(monster.gameObject);
            monsters.Remove(id);
        }
    }
}
```
上面示例里面的`ResourceManager.Load`采用同步方式加载资源。所以无论`CreateMonster`，`UpdateMonster`，`RemoveMonster`以怎样的顺序调用，都不会出现问题。


## 一个错误的异步版本

当我们采用`ResourceManager.LoadAsync`这样异步的方式来加载资源时，你可能会将`CreateMonster`修改为下面的实现。
```
public void CreateMonster(int id, MonsterData data)
{
	if(monsters.ContainsKey(id))
		return;

	ResourceManager.LoadAsync(data.prefabPath, (go) => {
		var monster = go.GetComponent<Monster>();
		monster.SetMonsterData(data);
		monsters[id] = monster;
	});	
}
```
稍微加以思考，你就能发现改成上面这种实现之后会存在以下一些问题：

*	因为`LoadAsync`的回调时间是不确定的，所以连续使用相同的参数调用多次`CreateMonster`会造成重复加载的问题。
*	对同一`id`，调用`CreateMonster`之后，接着立即调用`UpdateMonster`，可能会造成`UpdateMonster`不起作用。
*	对同一`id`，调用`CreateMonster`之后，接着立即调用`RemoveMonster`，可能无法真正删除这个怪物。


## 问题的所在

造成上面问题的直接原因，是`monsters.ContainsKey(id)`这个判断失去了作用。在`CreateMonster`中，只有异步回调的时候，才会向`monsters`里面添加数据。因此，这中间的不确定延时，便造成了上述的问题。


## 使用代理模式来解决这个问题

当然，此处我们不需要按照严格的代理模式来解决这个问题，我们可以借用一下代理的思想。创建一个类，通过这个类来管理加载的状态。
一个简单的实现如下。

```
class MonsterProxy
{
    public Monster MonsterObject { get; private set; }

    public MonsterData Data { get; private set; }

    private bool needDestroy;

    public MonsterProxy()
    {
        MonsterObject = null;
        Data = null;
        needDestroy = false;        
    }

    public void SetObject(Monster monster)
    {
        MonsterObject = monster;
        if(needDestroy)
        {
            DestroyObjectInternal();
            needDestroy = false;
        }

        if(MonsterObject != null && Data != null)
        {
            MonsterObject.SetMonsterData(Data);
        }
    }

    public void DestroyObject()
    {
        if(MonsterObject != null)
        {
            DestroyObjectInternal();
            needDestroy = false;
        }
        else
        {
            needDestroy = true;
        }
    }

    private void DestroyObjectInternal()
    {
        if(MonsterObject != null)
        {
            GameObject.Destroy(MonsterObject.gameObject);
            MonsterObject = null;
			Data = null;
        }
    }

    public void SetMonsterData(MonsterData data)
    {
		Data = data;
        if(MonsterObject != null)
        {            
            MonsterObject.SetMonsterData(Data);
        }
    }
}
```
然后对`MonsterManager`做一些修改。
```
class MonsterManager
{
    private Dictionary<int, MonsterProxy> monsters = new Dictionary<int, MonsterProxy>();

    public void CreateMonster(int id, MonsterData data)
    {
        if(monsters.ContainsKey(id))
            return;
        
        MonsterProxy proxy = new MonsterProxy();
        monsters[id] = proxy;

        ResourceManager.LoadAsync(data.prefabPath, (go) => {
            var monster = go.GetComponent<Monster>();
            proxy.SetObject(monster);
            proxy.SetMonsterData(data);
        });	
    }

    public void UpdateMonster(int id, MonsterData data)
    {
        if(monsters.ContainsKey(id))
            return;
        var proxy = monsters[id];
        proxy.SetMonsterData(data);
    }

    public void RemoveMonster(int id)
    {
        if(monsters.ContainsKey(id))
        {
            var proxy = monsters[id];
            proxy.DestroyObject();
            monsters.Remove(id);
        }
    }
}
```

## 抽象一下

可以从`MonsterProxy`中抽象出一组公用方法，来方便处理这一类的问题。

```
public abstract class ObjectProxy<T> where T: class
{
    public T CurrentObject { get; protected set; }

    private bool needDestroy;

    public ObjectProxy()
    {
        CurrentObject = null;
        needDestroy = false;        
    }

    public virtual void SetObject(T obj)
    {
        CurrentObject = obj;
        if(needDestroy)
        {
            DestroyObjectInternal();
            needDestroy = false;
        }
    }

    public virtual void DestroyObject()
    {
        if(CurrentObject != null)
        {
            DestroyObjectInternal();
            needDestroy = false;
        }
        else
        {
            needDestroy = true;
        }
    }

    protected abstract void DestroyObjectInternal()
}
```
然后`MonsterProxy`可以继承这个`ObjectProxy`。
```
class MonsterProxy: ObjectProxy<Monster>
{
    public MonsterData Data { get; private set; }

    protected override void SetObject(Monster data)
    {
        base.SetObject(data);

        if(CurrentObject != null && Data != null)
        {
            CurrentObject.SetMonsterData(Data);
        }
    }

    protected override void DestroyObjectInternal()
    {
        if(CurrentObject != null)
        {
            GameObject.Destroy(CurrentObject.gameObject);
            CurrentObject = null;
            Data = null;
        }
    }

    public void SetMonsterData(MonsterData data)
    {
        Data = data;
        if(CurrentObject != null)
        {            
            CurrentObject.SetMonsterData(Data);
        }
    }
}
```