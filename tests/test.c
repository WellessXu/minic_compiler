int array[110];
int n;
void init(int n) {
  int i;
  i = 1;
  while (i <= n * n + 1) {
    array[i] = -1;
    i = i + 1;
  }
}

// int getint();
// void putint(int k);
// void putch(int k);

int findfa(int a) {
  if (array[a] == a)
    return a;
  else {
    array[a] = findfa(array[a]);
    return array[a];
  }
}
void mmerge(int a, int b) {
  int m;
  int n;
  m = findfa(a);
  n = findfa(b);
  if (m != n) array[m] = n;
}
int main() {
  int t, m;
  int a, b;
  t = 1;
  while (t) {
    t = t - 1;
    n = 4;
    m = 10;
    int i;
    i = 0;
    int flag;
    flag = 0;
    init(n);
    int k;
    k = n * n + 1;

    while (i < m) {
      a = getint();
      b = getint();

      if (!flag) {
        int loc;
        loc = n * (a - 1) + b;

        array[loc] = loc;
        if (a == 1) {
          array[0] = 0;
          mmerge(loc, 0);
        }
        if (a == n) {
          array[k] = k;
          mmerge(loc, k);
        }
        if (b < n && array[loc + 1] != -1) {
          mmerge(loc, loc + 1);
        }
        if (b > 1 && array[loc - 1] != -1) {
          mmerge(loc, loc - 1);
        }
        if (a < n && array[loc + n] != -1) {
          mmerge(loc, loc + n);
        }
        if (a > 1 && array[loc - n] != -1) {
          mmerge(loc, loc - n);
        }

        if (array[0] != -1 && array[k] != -1 && findfa(0) == findfa(k)) {
          flag = 1;
          int tmp;
          tmp = i + 1;
          putint(tmp);
          putch(10);
        }
      }

      i = i + 1;
    }
    if (!flag) {
      putint(-1);
      putch(10);
    }
  }
  return 0;
}



if(a==1){
	b=1;
}else {if(a==2){
	b=2;}
}

if(a==1){
	b=1;
}
if(a==2){
	b=2;
}