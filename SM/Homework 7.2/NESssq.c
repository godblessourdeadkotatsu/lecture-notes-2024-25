/*ciao*/
typedef struct {
    int type;
    char name[256];
    double create_time;
    double occur_time;
    double arrival_time;
    double service_time;
} event_notice;
struct node{
    event_notice event;
    struct node* left_link;   // Pointer to the previous node (for doubly linked list)
    struct node* right_link;  // Pointer to the next node (for doubly linked list)
};