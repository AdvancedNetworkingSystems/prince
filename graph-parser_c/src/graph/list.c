
#include "graph/list.h"

/**
 * List constructor. It can works either as a queue or a stack based on function
 * called on it. It is user defined, not enforced.
 * It initializes parameters
 * @param q The list we want to initialize (either static or dynamic memory)
 */
void init_list(struct list *q)
{
	q->head = 0;
	q->tail = 0;
	q->size = 0;
}

/**
 * List insert. The new element is appended to list as if it is a queue.
 *
 * @param q The list or queue we want to expand with a new element
 * @param item The new item that will be inserted at the end.
 */
void enqueue_list(struct list *q, void *item)
{
	struct node_list *n =
		(struct node_list *)malloc(sizeof(struct node_list));


	n->content = item;

	if (q->head == 0) {
		n->prev = 0;
		n->next = 0;
		q->head = n;
		q->size = 1;
		q->tail = n;
	} else {
		n->prev = q->tail;
		q->tail->next = n;
		n->next = 0;
		q->tail = n;

		q->size++;
	}
}

/**
 * List remove. The element returned is the first one in the list, if the list
 * is empty, a 0 pointer is returned, in a FIFO fashion.
 *
 * @param q A list that contains the element
 * @return The first element in the queue, if present.
 * Null pointer if no one present.
 */
void *dequeue_list(struct list *q)
{
	void *ret_val = 0;

	if (q->head != 0) {
		struct node_list *to_remove = q->head;


		ret_val = q->head->content;

		if (q->head->next == 0) {
			q->head = 0;
			q->tail = 0;
		} else {
			q->head = q->head->next;
			q->head->prev = 0;
		}

		free(to_remove);
		q->size--;
	}

	return ret_val;
}

/**
 * List inspect. It inspects the list and returns the content of last element,
 * the tail one.
 *
 * @param q A list that are we inspecting
 * @return The last element, without removing it, if present.
 * Null pointer if no one present..
 */
void *peek_last_list(struct list *q)
{
	void *ret_val = 0;

	if (q->tail != 0) {
		ret_val = q->tail->content;
	}

	return ret_val;
}

/**
 * List inspect. It inspects the list and returns the content of first element,
 * the tail one.
 * @param q A list that are we inspecting
 * @return  The first element, without removing it, if present.
 * Null pointer if no one present.
 */
void *peek_first_list(struct list *q)
{
	void *ret_val = 0;

	if (q->head != 0) {
		ret_val = q->head->content;
	}

	return ret_val;
}

/**
 * List remove. The element returned is the last one in the list, if the list
 * is empty, a 0 pointer is returned, in a LIFO fashion. Works like a stack.
 *
 * @param q A list that contains the element
 * @return The last element in the queue, if present.
 * Null pointer if no one present.
 */
void *pop_list(struct list *q)
{
	void *ret_val = 0;

	if (q->tail != 0) {
		struct node_list *to_remove = q->tail;


		ret_val = to_remove->content;
		q->tail = to_remove->prev;

		if (q->tail != 0) {
			q->tail->next = 0;
		} else {
			q->head = 0;
		}

		free(to_remove);
		q->size--;
	}

	return ret_val;
}

/**
 * Helper function that print list pointers
 *
 * @param q A list
 */
void print_list(struct list *q)
{
	struct node_list *n = q->head;


	printf("[");

	while (n != 0) {
		printf("%p ", n);

		n = n->next;
	}

	printf("]\n");
}

/**
 * Given a list, it empties it and reset the list as if it is empty
 *
 * @param q A list we want to reuse or free its memory
 */
void clear_list(struct list *q)
{
	struct node_list *n = q->head;


	while (n != 0) {
		struct node_list *tmp = n;


		n = n->next;

		free(tmp);
	}

	q->head = 0;
	q->tail = 0;
	q->size = 0;
}

/**
 * It returns whether a list is empty or not
 *
 * @param q A list
 * @return Whether the list is empty
 */
int is_empty_list(struct list *q)
{
	return q->head == 0;
}

/**
 * Priority queue initializers.
 *
 * @param q The priority queue we want to initialize (either static or dynamic
 *  memory)
 */
void init_priority_queue(struct priority_queue *q)
{
	q->head = 0;
	q->tail = 0;
	q->size = 0;
}

/**
 *  Priority queue insertion.
 *  An item is inserted in the place defined by  @val, the value of the item.
 *  It implements increasing order.
 *  E.g. if @val is the lowest, item will be first.
 *
 * @param q A priority queue
 * @param item An item to be added
 * @param val The value that defines the position (the priority) of it
 */
void insert_priority_queue(struct priority_queue *q, void *item, double val)
{
	struct node_priority_queue *n = (struct node_priority_queue *)malloc(
		sizeof(struct node_priority_queue));


	n->content = item;
	n->value = val;

	if (q->head == 0) { // if priority list is empty
		n->prev = 0;
		n->next = 0;
		q->head = n;
		q->size = 1;
		q->tail = n;
	} else {
		struct node_priority_queue *n_current = q->head;


		while ((n_current != 0)
		       && (n_current->value
			   < val)) { // find the node we have to replace
			n_current = n_current->next;
		}

		if (n_current
		    == 0) { // if it is the last, we put the node as last
			n->prev = q->tail;
			q->tail->next = n;
			n->next = 0;
			q->tail = n;
		} else {			    // if it is not the last
			if (n_current->prev != 0) { // if it is not the first
				n_current->prev->next = n;
				n->prev = n_current->prev;
				n_current->prev = n;
				n->next = n_current;
			} else { // if it is the first
				q->head->prev = n;
				q->head = n;
				n->prev = 0;
				n->next = n_current;
			}
		}

		q->size++;
	}
}

/**
 * Priority queue insertion with update check.
 * It inserts an item if it is not present. If it is, the item is repositioned
 * to reflect its @val change.
 *
 *
 * @param q A priority queue
 * @param item An item to be added or present but update in value
 * @param val The value that defines the position (the priority) of it
 */
void insert_or_update_priority_queue(struct priority_queue *q, void *item,
				     double val)
{
	if (q->head == 0) { // if priority list is empty
		struct node_priority_queue *n =
			(struct node_priority_queue *)malloc(
				sizeof(struct node_priority_queue));


		n->content = item;
		n->value = val;
		n->prev = 0;
		n->next = 0;
		q->head = n;
		q->size = 1;
		q->tail = n;
	} else {
		struct node_priority_queue *n_current = q->head;


		struct node_priority_queue *to_replace = 0;


		struct node_priority_queue *actual_pos = 0;


		while ((n_current != 0)
		       && ((to_replace == 0)
			   || (actual_pos == 0))) { // stop if we end queue
			if (n_current->content == item) {
				actual_pos = n_current;
			}

			if ((n_current->value > val) && (to_replace == 0)) {
				to_replace = n_current;
			}

			n_current = n_current->next;
		}

		struct node_priority_queue *n = 0;


		if (actual_pos != 0) { // if node is already in queue
			if (actual_pos == to_replace) {
				actual_pos->value = val;

				return;
			}

			n = actual_pos;
			actual_pos->value = val;

			if (actual_pos->prev != 0) {
				actual_pos->prev->next = actual_pos->next;
			} else {
				q->head = actual_pos->next;
			}

			if (actual_pos->next != 0) {
				actual_pos->next->prev = actual_pos->prev;
			} else {
				q->tail = actual_pos->prev;
			}
		} else { // if node is new, do the same as insert, except we
			 // already have address
			n = (struct node_priority_queue *)malloc(
				sizeof(struct node_priority_queue));
			n->content = item;
			n->value = val;

			q->size++;
		}

		if (to_replace
		    == 0) { // if it is the last, we put the node as last
			n->prev = q->tail;
			q->tail->next = n;
			n->next = 0;
			q->tail = n;
		} else {			     // if it is not the last
			if (to_replace->prev != 0) { // if it is not the first
				to_replace->prev->next = n;
				n->prev = to_replace->prev;
				to_replace->prev = n;
				n->next = to_replace;
			} else { // if it is the first
				q->head->prev = n;
				q->head = n;
				n->prev = 0;
				n->next = to_replace;
			}
		}
	}
}

/**
 * Priority queue remove.
 * Given a priority queue, it returns the element with highest priority (i.e.
 * with lowest value). This is equivalent to a dequeue, since elements are
 * stored already sorted.
 * If queue is empty, a 0 pointer is returned
 *
 * @param q A priority queue
 * @return The first element of the priority queue, or 0 if it is empty.
 */
void *dequeue_priority_queue(struct priority_queue *q)
{
	void *ret_val = 0;

	if (q->head != 0) {
		struct node_priority_queue *to_remove = q->head;


		ret_val = q->head->content;

		if (q->head->next == 0) {
			q->head = 0;
			q->tail = 0;
		} else {
			q->head = q->head->next;
			q->head->prev = 0;
		}

		q->size--;
		free(to_remove);
	}

	return ret_val;
}

/**
 * Helper function that print priority queue pointers
 *
 * @param q A priority queue
 */
void print_priority_queue(struct priority_queue *q)
{
	struct node_priority_queue *n = q->head;


	printf("[");

	while (n != 0) {
		printf("%f ", n->value);

		n = n->next;
	}

	printf("]\n");
}

/**
 * It returns whether a priority queue is empty or not
 *
 * @param q A priority queue
 * @return Whether the priority queue is empty
 */
int is_empty_priority_queue(struct priority_queue *q)
{
	return q->head == 0;
}
