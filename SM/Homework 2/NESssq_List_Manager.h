

nodePtr get_new_node();
void  return_node(nodePtr item);
nodePtr pop_from_AL(void);
void push_on_AL(nodePtr item);
void schedule(nodePtr new_node_event);
nodePtr event_pop(void);
void print_FEL();
void print_list(dll * doublell);
void enqueue(nodePtr new_node, dll * curr_queue);
nodePtr dequeue(dll * curr_queue);
void destroy_list(nodePtr p);

