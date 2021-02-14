#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2
#define GROUP_A 0
#define GROUP_B 1

// partitioning node used for reordering points and accounting values
struct reorder_accnt_divider_node{
   void *next;
   void *prior;
   float loc;
   float value;
   float groupA_prop; // propagation value
   float groupB_prop; // propagation value
   char group; // 0 == A, 1 == B
};

void corr3d_freeNodes(struct reorder_accnt_divider_node *);
void corr3d_createNodes(struct reorder_accnt_divider_node **, int);
void corr3d_copyData(char, float *, float *, int, char, struct reorder_accnt_divider_node *);
void corr3d_reorder(struct reorder_accnt_divider_node **);
void corr3d_account(struct reorder_accnt_divider_node *);
float corr3d_correspondenceVal(struct reorder_accnt_divider_node *,
                               struct reorder_accnt_divider_node *,
                               struct reorder_accnt_divider_node *);
