f(x)
     unsigned x;
{
  static short c;
  return x>>c;
}
g(x)
     unsigned x;
{
  static char c;
  return x>>c;
}
