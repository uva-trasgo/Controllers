N_PRIORITIES = 5

times = [0.1, 0.5, 0.8]

#n_swaps = []


if __name__ == "__main__":

    for size in [200, 300, 400, 500, 600]:
        n_swaps = []
        for p in range(N_PRIORITIES):
            line_times = []

            print(f"Max(Q_{p}) & ", end="")

            for mode in ["no-preemption", "preemption"]:
                for i in range(len(times)):

                    f = open(f"stats/serving_times-{mode}-{times[i]}-{size}.txt")
                    #serving_times-preemption-0_0.8-600.txt
                    # Skip previous priorities lines
                    for j in range(p):
                        f.readline()

                    line_times.append(int(f.readline().strip().split()[0]))

                    while f.readline().strip() != "===":
                        pass

                    while f.readline().strip() != "===":
                        pass

                    n_swaps.append(int(f.readline()))

            for j in range(len(line_times)-1):
                print(f"{line_times[j]} & ", end="")

            print(f"{line_times[-1]} \\\\")

        print("Swaps & ", end="")
        for i in range(len(line_times)-1):
            print(f"{n_swaps[i]} & ", end="")
        print(f"{n_swaps[len(line_times)-1]}")

