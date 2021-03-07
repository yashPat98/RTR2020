// --- Headers ---
#include "DataStructure.h"
#include "Destiny.h"

// --- CreateList() --- Creates linked list of required size with random data ---
struct Node *CreateList(struct Node *head, int size)
{
    //variable declaration
    struct Node *temp = NULL;
    struct Node *ptr = NULL;
    int i;

    //code
    for(i = 0; i < size; i++)
    {
        temp = (struct Node *)malloc(sizeof(struct Node));
        if(temp == NULL)
        {
            //free memory
            while(i >= 0)
            {
                temp = head;
                head = head->next;
                if(temp)
                    free(temp);

                i--;
            }
        }

        //random position of trees
        temp->x = (float)((rand() % 400) - 200) * 0.1f;
        temp->y = 0.0f;
        temp->z = (float)((rand() % 400) - 200) * 0.1f;

        if(head == NULL)
        {
            temp->next = NULL;
            head = temp;
        }
        else
        {
            ptr = head;
            while(ptr->next != NULL)
                ptr = ptr->next;

            ptr->next = temp;
            temp->next = NULL;
        }
    }

    temp = NULL;
    ptr = NULL;
    return (head);
}

struct Node *DeleteElementsInRange(struct Node *head, float xLower, float xUpper, float zLower, float zUpper)
{
    //variable declaration
    struct Node *ptr = NULL;
    struct Node *temp1 = NULL;
    struct Node *temp2 = NULL;

    //code
    ptr = head;
    while(ptr != NULL)
    {
        if(ptr->x >= xLower && ptr->x <= xUpper &&
           ptr->z >= zLower && ptr->z <= zUpper)
        {
            //delete element
            if(ptr == head)
            {
                head = head->next;
                if(ptr)
                    free(ptr);
                    
                ptr = head;
            }
            else
            {
                temp1->next = ptr->next;
                temp2 = ptr;
                ptr = ptr->next;

                if(temp2)
                    free(temp2);
            }
                
        }
        else
        {
            temp1 = ptr;
            ptr = ptr->next;
        }
    }

    ptr = NULL;
    temp1 = NULL;
    temp2 = NULL;
    return (head);
}

struct Node *DeleteList(struct Node *head)
{
    //variable declaration
    struct Node *ptr = NULL;

    //code
    if(head != NULL)
    {
        ptr = head;
        while(ptr != NULL)
        {
            head = head->next;
            if(ptr)
                free(ptr);
            ptr = head;
        }
    }

    ptr = NULL;
    head = NULL;
    return (head);
}

