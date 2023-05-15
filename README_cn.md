# 向量余弦相似度计算

## Build

```bash
$> mkdir build
$> cd build
$> cmake ../
$> make
```
运行上面的命令，通过 `cmake` 完成编译，生成 `calc_cosim` 可执行文件。

## Usage

你可以使用 `./calc_cosim --help` 查看运行参数。简要的说明如下：
<!--
|Flags| Describe | default |
|--|--|--|
|--dict |dictionary vectors file path | "dict_vec.csv"
|--seed |seed vectors file path | "seed_vec.csv"
|--round |the number of Round of the current execution | 1
-->

```
--seed 
	string, seed vectors file path. Default: "seed_vec.csv'.
--dict
    string, dictionary vectors file path. Default: "dict_vec.csv'.
--round
    int32, the number of Round of the current execution. Default: 1.
```
比如，`./calc_cosim --seed ./test/seed_vec.csv --dict ./test/dict_vec.csv --round 1`  表示读取 `./test` 目录下的 `seed_vec.csv` 和 `dict_vec.csv` 进行第一轮次运行。

轮次号主要用于 `duration.log` 中记录运行时间。

`result.csv` 和 `duration.log` 文件会在运行后在当前目录生成。
