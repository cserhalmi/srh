#include "math.h"

Math::Math()
{
}

int Math::min(int a, int b)
{
  if (a<b) return a; else return b;
}

int Math::max(int a, int b)
{
  if (a>b) return a; else return b;
}

int Math::limit(int v, int l, int u)
{
  if (v>u) return u;
  else if (v<l) return l;
  else return v;
}

