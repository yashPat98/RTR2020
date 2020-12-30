// --- Linked List Header ---
struct Node
{
    float x, y, z;
    struct Node *next;
};

struct Node *CreateList(struct Node *head, int size);
struct Node *DeleteList(struct Node *head);
struct Node *DeleteElementsInRange(struct Node *head, float xLower, float xUpper, float zLower, float zUpper);
