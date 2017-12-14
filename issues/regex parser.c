


check_char(s, p)
   switch(p)
      case '.':
         return true;
      default:
         if (s != p)
            return false;
         else
            return true;
   return true; // compiler happy


isMatch(char *str, char *pattern)
   j = 0;
   star_count = 0;
   k = 0;
   pattern_len = strlen(pattern)
   str_len = strlen(str)

   for i = 0 to str_len - 1
      if (pattern[j] == '*')
         i--;
         j++;
         continue;

      if (j < pattern_len && pattern[j + 1] == '*')
         push(backtracks, [i, j, star_count + 1]) // some changes have to be done here
         for k = 0 to star_count - 1
            if (!check_char(str[i++], pattern[j]))
               return false
         star_count = 0;
         i--;
         j++;
      else if (check_char(str[i], pattern[j]))
         j++;
         continue;
      else
         node = pop(backtracks)
         if (!node)
            return false;
         i = node.i - 1
         j = node.j
         star_count = node.next_star_count

   if (pattern_len == (j + 1) && i == str_len)
      return true;
   else
      return false;


isMatch("aab", "c*a*b")
i = 2
j = 4
star_count = 0

stack:
[0, 2, 3]
[0, 0, 1]
