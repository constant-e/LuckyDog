# 配置文件说明
配置文件模板/默认配置文件：`docs/config_template.json`

## `t`
数据类型：数字  
默认值：`100`  
描述：抽取学生时的时间间隔（单位：毫秒）

## `nameList`
数据类型：数组  
默认值：
```json
[
    {"id":1, "name":"学生1", "w":10},
    {"id":2, "name":"学生2", "w":10}
]
```
描述：学生列表。数组中的对象应包含：
1. `id`：学号，数字类型
2. `name`：姓名，字符串类型
3. `w`：权重，数字类型（应为正数）
