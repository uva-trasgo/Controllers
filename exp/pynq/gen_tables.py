from process_data import gen_tasks, Task, Swap, gen_avg_service_times, gen_preemption_overhead, gen_overall_overhead
import numpy as np
import pandas as pd
from SyncRNG import SyncRNG
from itertools import product
import os

def gen_avg_preemption_overheads():
    seeds = [28871727]
   
    s = SyncRNG(seed=seeds[0]) 
    for i in range(9):
        seeds.append(s.randi())

    mean_overheads_rr = []
    std_overheads_rr = []

    mean_overheads_global = []

    ov = -1

    for rr in [1]:
        mean_overheads = []
        for seed in seeds:
            ov = gen_preemption_overhead(seed, rr)
            print(ov)
            if ov >= 0:
                mean_overheads.append(ov)
                mean_overheads_global.append(ov)

        mean_ov = np.mean(mean_overheads)
        std_ov = np.std(mean_overheads)

        mean_overheads_rr.append(mean_ov)
        std_overheads_rr.append(std_ov)

        global_mean_ov = np.mean(mean_overheads_global)
        global_std_ov = np.std(mean_overheads_global)

    print("RR\tmean_ov (%)\tstd_ov (%)")
    print(f"1\t{mean_overheads_rr[0]*100:.2f}\t{std_overheads_rr[0]*100:.2f}")
    #print(f"2\t{mean_overheads_rr[1]*100:.2f}\t{std_overheads_rr[1]*100:.2f}")

    print("Global preemption overhead\n------------------------")
    print("mean (%)\tstd (%)")
    print(f"{global_mean_ov*100}\t{global_std_ov*100}")
    #print(mean_overheads_rr)
    #print(std_overheads_rr)

def gen_avg_overall_overhead():
    seeds = [28871727]
   
    s = SyncRNG(seed=seeds[0]) 
    for i in range(9):
        seeds.append(s.randi())

    mean_ov = []
    std_ov = []

    for seed in seeds:
        mean, std = gen_overall_overhead(seed)
        mean_ov.append(mean)
        std_ov.append(std)

    overall_mean_ov = np.mean(mean_ov)
    overall_std_ov = np.std(std_ov) 

    print(f"Overall overhead (%): {overall_mean_ov*100} +- {overall_std_ov*100}")


def transform_plusminus(orig_df):
        even_cols = orig_df.columns[1::2]
        odd_cols = orig_df.columns[2::2]

        for i in range(len(even_cols)):
            orig_df[even_cols[i]] = orig_df[even_cols[i]].map('{:.2f}'.format) + "±" + orig_df[odd_cols[i]].map('{:.2f}'.format)
        #df =  df[odd_cols]
        df = pd.concat([orig_df['seed'], orig_df[even_cols]], axis=1)

        return df

if __name__ == "__main__":

    seeds = [28871727]
    s = SyncRNG(seed=seeds[0]) 
    n_seeds = 10
    for i in range(n_seeds - 1):
        seeds.append(s.randi())    


    RR = [1,2]
    SIZE = [600]
    P = [1]
    M = [0,1]
    TIMES = [0.1, 0.5, 0.8]
    PARTIAL = [1]



    avg_priority_service_table_df = {}
    avg_p0_service_table_df = {}
    avg_p4_service_table_df = {}
    
    for rr in RR:
        avg_priority_service_table_df[rr] = pd.DataFrame(columns=['seed', 'avg(Bnp)', 'std(Bnp)', 'avg(Mnp)', 'std(Mnp)', 'avg(Inp)', 'std(Inp)', 'avg(Bp)', 'std(Bp)', 'avg(Mp)', 'std(Mp)', 'avg(Ip)', 'std(Ip)'])

        avg_p0_service_table_df[rr] = pd.DataFrame(columns=['seed', '$\overline{B_{np}}$', '$\overline{M_{np}}$', '$\overline{I_{np}}$', '$\overline{B_p}$', '$\overline{M_p}$', '$\overline{I_p}$'])

        avg_p4_service_table_df[rr] = pd.DataFrame(columns=['seed', '$\overline{B_{np}}$', '$\overline{M_{np}}$', '$\overline{I_{np}}$', '$\overline{B_p}$', '$\overline{M_p}$', '$\overline{I_p}$'])

        for seed in seeds:
            seed_row_avg = []
            seed_row_p0 = []
            seed_row_p4 = []

            avg_service_times_df = gen_avg_service_times(seed, rr)


            for params in product([rr], SIZE, P, M, TIMES, PARTIAL):
                avg = np.mean(avg_service_times_df[params])
                std = np.std(avg_service_times_df[params])

                seed_row_avg.append(avg)
                seed_row_avg.append(std)
                seed_row_p0.append(avg_service_times_df[params][0])
                seed_row_p4.append(avg_service_times_df[params][4])


            adapted_row_pall = [seed] + seed_row_avg
            adapted_row_p0 = [seed] + seed_row_p0
            adapted_row_p4 = [seed] + seed_row_p4
            #print(f"adapted_row: {adapted_row_pall}")
            avg_priority_service_table_df[rr].loc[len(avg_priority_service_table_df[rr])] = adapted_row_pall
            avg_p0_service_table_df[rr].loc[len(avg_p0_service_table_df[rr])] = adapted_row_p0
            avg_p4_service_table_df[rr].loc[len(avg_p4_service_table_df[rr])] = adapted_row_p4

        print(f"N_RR: {rr}")
        avg_priority_service_table_df[rr]['seed'] = avg_priority_service_table_df[rr]['seed'].astype('int') 
        avg_p0_service_table_df[rr]['seed'] = avg_p0_service_table_df[rr]['seed'].astype('int')
        avg_p4_service_table_df[rr]['seed'] = avg_p4_service_table_df[rr]['seed'].astype('int')


        try:
            os.makedirs("../../../paper/tables")
        except:
            pass

        f = open(f"../../../paper/tables/avg_service_time-{rr}rr-allp.tex", "w")
        df = avg_priority_service_table_df[rr]
        df = transform_plusminus(df)
        #f.write(avg_priority_service_table_df[rr].style.hide(axis="index").format(precision=2).to_latex())
        f.write(df.style.hide(axis="index").to_latex(caption=f"Average service time for all priorities with standard deviation for 30 tasks scheduled on {rr} RRs.", column_format="ccccccc", position_float="centering", hrules=True))
        f.close()
        print(df)
        #print(avg_priority_service_table_df[rr].style.hide(axis="index").format(precision=2).to_latex())
        #df = df.groupby(df.columns.str[-1], axis=1).apply(lambda x: x.astype(str).apply('±'.join, 1))


        f = open(f"../../../paper/tables/avg_service_time-{rr}rr-p0.tex", "w")
        df = avg_p0_service_table_df[rr]
        #df = transform_plusminus(df)
        #f.write(avg_p0_service_table_df[rr].style.hide(axis="index").to_latex())
        f.write(df.style.hide(axis="index").to_latex(caption=f"Average service time for maximum priority with standard deviation for 30 tasks scheduled on {rr} RRs.", column_format="ccccccc", position_float="centering", hrules=True, label=f"table:service_time_p0_{rr}"))
        f.close()
        print(df)
        #print(avg_p0_service_table_df[rr].style.hide(axis="index").to_latex())

        f = open(f"../../../paper/tables/avg_service_time-{rr}rr-p4.tex", "w")
        df = avg_p4_service_table_df[rr]
        #df = transform_plusminus(df)
        #f.write(avg_p0_service_table_df[rr].style.hide(axis="index").to_latex())
        f.write(df.style.hide(axis="index").to_latex(caption=f"Average service time for minimum priority with standard deviation for 30 tasks scheduled on {rr} RRs.", column_format="ccccccc", position_float="centering", hrules=True, label=f"table:service_time_p4_{rr}"))
        f.close()
        print(df)
        #print(avg_p0_service_table_df[rr].style.hide(axis="index").to_latex())

        gen_avg_preemption_overheads() 
