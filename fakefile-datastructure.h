typedef struct fake {

  struct rule *rules;
  int num_of_rules;
  
} Fakefile;

typedef struct rule {

  struct rule *next_rule;  
  char **target_and_deps;
  char **commands;
  int num_rule;

} Fakefile_Rule;
