

// FIXME: wrap in one of those preprocessor "templates"
int32_t
round_up_integer(int32_t value, int32_t roundto)
{
  if (value % roundto == 0)
  {
    return value;
  }
  else
  {
    return roundto * (1 + value/roundto);
  }
}
