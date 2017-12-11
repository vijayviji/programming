#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#include "../common.h"

typedef enum TokenType {
   TOKEN_INVALID = 0,
   TOKEN_FILE,
   TOKEN_DIR,
} TokenType;

typedef struct Token {
   char *name;
   TokenType type;
   uint32_t len;
   uint32_t level;
} Token;

typedef Token * TokenHandle;

typedef struct LevelLengths {
   uint32_t *cur_path_len_at_level;
   uint32_t max_length;
   uint32_t num_levels;
} LevelLengths;

LevelLengths * init_token_levels(uint32_t levels)
{
   LevelLengths *level_lengths;

   level_lengths = malloc(sizeof(LevelLengths));
   if (!level_lengths) {
      return NULL;
   }

   level_lengths->cur_path_len_at_level = malloc(sizeof(uint32_t) * levels);
   if (!level_lengths->cur_path_len_at_level) {
      goto free_level_lengths;
   }
   memset(level_lengths->cur_path_len_at_level, 0, sizeof(uint32_t) * levels);

   level_lengths->max_length = 0;
   level_lengths->num_levels = levels;
   return level_lengths;

free_level_lengths:
   free(level_lengths);
   return NULL;
}

void init_token(Token *token)
{
   token->level = 0;
   token->len = 0;
   token->name = NULL;
   token->type = TOKEN_INVALID;
}

Token *read_next_token(Token *token,             // OUT
                       char *input,              // IN
                       uint32_t *start_index)    // IN/OUT
{
   char letter = '\0';
   uint32_t i = 0;

   init_token(token);

   for(i = *start_index, letter = input[*start_index];
       letter != '\n' && letter != '\0';
       (*start_index)++, letter = input[*start_index]) {
      if (letter == '\t') {
         token->level++;
      } else {
         if (!token->name) {
            token->name = &input[*start_index];
         }
         if (letter == '.') {
            token->type = TOKEN_FILE;
         }
         token->len++;
      }
   }

   if (letter == '\n') {
      (*start_index)++;
   }

   if (!token->name) {
      assert(token->len == 0);
      return NULL;
   }

   //printf("%c-%d\n", token->name[0], token->len);

   if (token->type == TOKEN_INVALID) {
      token->type = TOKEN_DIR;
   }

   return token;
}

uint32_t lengthLongestPath(char* input) {
   Token token;
   uint32_t start_index = 0, full_path_len = 0, parent_path_len = 0;
   uint32_t input_len = strlen(input), i;
   LevelLengths *level_lengths = NULL;

   if (!input_len) {
      return 0;
   }

   level_lengths = init_token_levels(input_len / 2);
   if (!level_lengths) {
      return 0;
   }

   while(1) {
      read_next_token(&token, input, &start_index);

      if (!token.len) {
         break;
      }

      for(i = 0; i < token.level; i++) {
         printf("\t");
      }
      printf("%s (%u)\n", substring(token.name, 0, token.len), token.len);

      if (token.level > 0) {
         // +1 for '/' in dir/
         parent_path_len = level_lengths->cur_path_len_at_level[token.level - 1] + 1;
      } else {
         parent_path_len = 0;
      }

      if (token.type == TOKEN_DIR) {
         level_lengths->cur_path_len_at_level[token.level]
            = parent_path_len + token.len;
      } else {

         full_path_len = parent_path_len + token.len;

         if (full_path_len > level_lengths->max_length) {
            level_lengths->max_length = full_path_len;
         }
      }
   }

   return level_lengths->max_length;
}


int main()
{
   char *input = "a\n\tb.txt\na2\n\tb2.txt";

   printf("Longest path len: %u\n", lengthLongestPath(input));
}
