# INSTALLATION

## BUILD

```
$> cd ./src
$> bash ./build.sh
```

## RUN

```
$> cd ./src
$> ./a.out -seed './path/seed_vld.csv'a -dict './path/dict_vld.csv' -round 5
```

数据文件的默认值是 `seed_vec.csv` 和 `dict_vec.csv`。如果这两个文件和编译生成的文件 `a.out` 在同一个目录下，可以直接运行：

```
$> ./a.out
```
-round 参数指示运行轮次。默认值是1。

运行后在当前目录下生成 `result.csv` 和 `duration.log` 文件。
