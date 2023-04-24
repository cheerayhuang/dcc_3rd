import numpy as np
import time


def get_cos_similar_multi(v1: list, v2: list):
    num = np.dot([v1], np.array(v2).T)  # 向量点乘
    denom = np.linalg.norm(v1) * np.linalg.norm(v2, axis=1)  # 求模长的乘积
    res = num / denom
    res[np.isneginf(res)] = 0
    #return 0.5 + 0.5 * res
    return res


def main():

    #v1 = [0.2, 0.3]
    #v2 = [[0.15, 0.22], [0.11, 0.33], [0.23, 0.27]]

    v1 = []
    with open('seed_vec.csv', 'r') as f:
        v_str = f.readline()
        v1 = [float(x) for x in v_str.split(',')]
        #print(v1)

    v2 = []
    total_time = 0
    result = []
    with open('dict_vec.csv', 'r') as f:

        v_str = f.readline()
        v = [float(x) for x in v_str.split(',')]
        v2.append(v)

        i = 1
        while v_str is not None:
            v_str = f.readline()
            if v_str == '':
                break
            try:
                v = [float(x) for x in v_str.split(',')]
            except ValueError as e:
                print(f'read cur line: {v_str}')
                print(f'cur_i: {i}')
                raise e

            v2.append(v)

            i += 1
            if i % 10000 == 0:
                start = time.time_ns()
                res = get_cos_similar_multi(v1, v2)

                max_v = np.max(res)
                max_aix = np.argmax(res)
                result.append((max_v, max_aix + (i/10000-1) * 10000))

                end = time.time_ns()

                total_time += end-start
                v2 = []

                continue

    print(f'elapsed: {total_time}ns')
    print(f'elapsed: {total_time/1000.0}us')
    print(f'elapsed: {total_time/1000000.0}ms')
    print(f'elapsed per calc: {total_time/1000000.0}ns')
    result_val = [x for (x, _) in result]
    result_aix = [x for (_, x) in result]
    print(result_val)
    print(result_aix)


if __name__ == '__main__':
    main()

