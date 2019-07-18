
/* COPYRIGHT C 1991- Ali Dasdan */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "ad_defs.h"
#include "ad_fileio.h"
#include "ad_readinput.h"

/* initialize cells array */
void init_cells(int nocells, cells_t cells[])
{
    for (int i = 0; i < nocells; i++) { //节点数
        cells[i].cno_nets = 0;  //节点的总边数
        cells[i].cno_inets = 0; //在划分内的遍数
        cells[i].netlist = NIL; //指向节点边的指针
    }   /* for */
}   /* init_cells */

/* initialize netlist pointers */
void init_netlist(int nonets,
                  cells_t cells[], //节点信息数组
                  nets_t nets[],   //边信息数组（权值、两端点）
                  corn_t cnets[])  //节点或边的数量数组
{
    for (int i = 0; i < nonets; i++) { //边数
        /* for cell1 and cell2 */
        for (int j = 0; j < 2; j++) {  //两端点
            int tcell_no = nets[i].ncells[0]; //左端点（值为节点数组中的位置）
            if (j != 0) {
                tcell_no = nets[i].ncells[1]; //右端点
            }
            int net_index = cells[tcell_no].netlist + cells[tcell_no].cno_inets; //该节点的指针值+其划分内的边数
            cnets[net_index].corn_no = i;//应该不是FM算法
            cells[tcell_no].cno_inets++;
            if (cells[tcell_no].cno_inets > cells[tcell_no].cno_nets) {
                printf("Error: Inconsistency in cell_%d degrees.\n", j);
                exit(1);
            }   /* if */
        }   /* for j */
    }   /* for i */
}   /* init_netlist */

/* read input graph size to allocate memory */
void read_graph_size(char fname[],
                     int  *nocells,
                     int  *nonets)
{
    FILE *fp;

    open_file(&fp, fname, "r");

    if (fscanf(fp, "%d%d", nocells, nonets) == EOF) {
        printf("Error: Cannot read from %s: errno= %d error= %s\n", fname, errno, strerror(errno));
        close_file(&fp);
        exit(1);
    } 

    if ((*nocells < 0) || (*nonets < 0)) {
        printf("Error: Invalid attributes of graph.\n");
        close_file(&fp);
        exit(1);
    }   /* if */

    close_file(&fp);
}   /* read_graph_size */

/* read input graph and construct cell, net arrays */
void read_graph(char fname[],
                int  nocells,
                int  nonets,
                int  noparts,
                int *totsize,
                int *totcellsize,
                int *max_density,
                int *max_cweight,
                int *max_nweight,
                cells_t cells[],
                nets_t  nets[],
                corn_t  cnets[]) //数量数组（边/节点）
{
    FILE *fp;
    open_file(&fp, fname, "r");

    /* graph size is already read so re-read and discard. */
    int ignore1, ignore2;
    if (fscanf(fp, "%d%d", &ignore1, &ignore2) == EOF) {
        printf("Error: Cannot read from %s: errno= %d error= %s\n", fname, errno, strerror(errno));
        close_file(&fp);
        exit(1);
    } 

    /* initialize cells array */
    init_cells(nocells, cells);

    /* read nets */
    *max_nweight = -1;
    *totsize = 0;
    for (int i = 0; i < nonets; i++) {
        int ignore;  /* this is the number of pins on this net; it
                        makes sense for hypergraphs; for graphs, it is
                        always 2. */
        if (fscanf(fp, "%d%d%d%d", &nets[i].nweight, &ignore, &nets[i].ncells[0], &nets[i].ncells[1]) == EOF) {
            printf("Error: Cannot read from %s: errno= %d error= %s\n", fname, errno, strerror(errno));
            close_file(&fp);
            exit(1);
        }
        /* temp for reading 2, the net degree */
        cells[nets[i].ncells[0]].cno_nets++;
        cells[nets[i].ncells[1]].cno_nets++;
        *totsize += nets[i].nweight;
        if (nets[i].nweight > (*max_nweight)) {
            *max_nweight = nets[i].nweight;
        }
    }   /* for */

    /* set netlist pointers */
    *max_density = -1;
    *max_cweight = -1;
    *totcellsize = 0;
    int part_sum = 0;
    for (int i = 0; i < nocells; i++) {
        if (fscanf(fp, "%d", &cells[i].cweight) == EOF) { //读入边权值
            printf("Error: Cannot read from %s: errno= %d error= %s\n", fname, errno, strerror(errno));
            close_file(&fp);
            exit(1);
        }
        (*totcellsize) += cells[i].cweight;
        if (cells[i].cweight > (*max_cweight)) {  //计算maxNET 
            *max_cweight = cells[i].cweight;
        }
        if (cells[i].cno_nets > (*max_density)) { 
            *max_density = cells[i].cno_nets;
        }
        cells[i].netlist = part_sum;
        part_sum += cells[i].cno_nets;
    }   /* for */

    close_file(&fp);

    /* create netlists */
    init_netlist(nonets, cells, nets, cnets);
}   /* read_graph */

/* EOF */

