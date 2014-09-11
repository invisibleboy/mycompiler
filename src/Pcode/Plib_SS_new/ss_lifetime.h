#ifndef SS_LIFETIME_H
#define SS_LIFETIME_H

typedef enum
{
  FULL             = 0,
  FULL_PTR         = 1,
  
  LOOP             = 2,
  LOOP_PTR         = 3,

  DISJOINT_LTS     = 4,
  DISJOINT_LTS_PTR = 5,

  MERGE_PHIS       = 6,
  MERGE_PHIS_PTR   = 7,
}
PSS_LifetimeSelect;

#define LT_PTR(a) ((a) & 0x1)

extern int PSS_ComputeLifetimes (PC_Graph cfg, PSS_BaseTbl, PSS_LifetimeSelect);

#endif
