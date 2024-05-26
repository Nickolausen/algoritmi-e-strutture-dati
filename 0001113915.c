/*
 * > Author: Nicholas Magi, 0001113915, classe A, <nicholas.magi@studio.unibo.it>
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>

typedef struct Coordinates {
    int row;
    int col;
} Coordinates;

typedef struct Edge {
    long src;
    long dst;
    double weight;
    struct Edge *next;
} Edge;

typedef struct Node {
    int id;
    long height;
    double cost_from_src;

    Coordinates coords;
    Edge *links[4];

    struct Node *parent;
    struct Node *next;
} Node;

typedef struct Graph {
    int nr_nodes;
    Node *nodes;
} Graph;


typedef struct HeapElem {
    int key;
    double prio;
} HeapElem;

typedef struct {
    HeapElem *heap;
    int *pos; 
    int n; /* quante coppie (chiave, prio) sono effettivamente presenti nello heap */
    int size; /* massimo numero di coppie (chiave, prio) che possono essere contenuti nello heap */
} MinHeap;

typedef enum Direction { UP, RIGHT, DOWN, LEFT } Direction;

void minheap_print(const MinHeap *h)
{
    int i, j, width = 1;

    assert(h != NULL);

    printf("\n** Contenuto dello heap:\n\n");
    printf("n=%d size=%d\n", h->n, h->size);
    printf("Contenuto dell'array heap[] (stampato a livelli):\n");
    i = 0;
    while (i < h->n) {
        j = 0;
        while (j<width && i < h->n) {
            printf("h[%2d]=(%2d, %6.2f) ", i, h->heap[i].key, h->heap[i].prio);
            i++;
            j++;
        }
        printf("\n");
        width *= 2;
    }
    printf("\nContenuto dell'array pos[]:\n");
    for (i=0; i<h->size; i++) {
        printf("pos[%d]=%d ", i, h->pos[i]);
    }
    printf("\n\n** Fine contenuto dello heap\n\n");
}

void minheap_clear( MinHeap *h )
{
    int i;
    assert(h != NULL);
    for (i=0; i<h->size; i++) {
        h->pos[i] = -1;
    }
    h->n = 0;
}

/* Costruisce un min-heap vuoto che può contenere al massimo
   `size` elementi */
MinHeap *minheap_create(int size)
{
    MinHeap *h = (MinHeap*)malloc(sizeof(*h));
    assert(h != NULL);
    assert(size > 0);

    h->size = size;
    h->heap = (HeapElem*)malloc(size * sizeof(*(h->heap)));
    assert(h->heap != NULL);
    h->pos = (int*)malloc(size * sizeof(*(h->pos)));
    assert(h->pos != NULL);
    minheap_clear(h);
    return h;
}

void minheap_destroy( MinHeap *h )
{
    assert(h != NULL);

    h->n = h->size = 0;
    free(h->heap);
    free(h->pos);
    free(h);
}

/* Funzione di supporto: restituisce 1 sse l'indice `i` appartiene
   all'intervallo degli indici validi degli elementi validi nell'array
   che rappresenta lo heap. */
static int valid(const MinHeap *h, int i)
{
    assert(h != NULL);

    return ((i >= 0) && (i < h->n));
}

/* Funzione di supporto: scambia heap[i] con heap[j] */
static void swap(MinHeap *h, int i, int j)
{
    HeapElem tmp;

    assert(h != NULL);
    assert(valid(h, i));
    assert(valid(h, j));
    assert(h->pos[h->heap[i].key] == i);
    assert(h->pos[h->heap[j].key] == j);

    tmp = h->heap[i];
    h->heap[i] = h->heap[j];
    h->heap[j] = tmp;

    h->pos[h->heap[i].key] = i;
    h->pos[h->heap[j].key] = j;
}

/* Funzione di supporto: restituisce l'indice del padre del nodo i */
static int parent(const MinHeap *h, int i)
{
    assert(valid(h, i));

    return (i+1)/2 - 1;
}

/* Funzione di supporto: restituisce l'indice del figlio sinistro del
   nodo `i`. Ritorna un indice non valido se `i` non ha figlio
   sinistro. */
static int lchild(const MinHeap *h, int i)
{
    assert(valid(h, i));

    return 2*i + 1;
}

/* Funzione di supporto: restituisce l'indice del figlio destro del
   nodo `i`. Ritorna un indice non valido se `i` non ha figlio
   destro. */
static int rchild(const MinHeap *h, int i)
{
    assert(valid(h, i));

    return 2*i + 2;
}

/* Funzione di supporto: restituisce l'indice del figlio di `i` con
   priorità minima. Se `i` non ha figli, restituisce -1 */
static int min_child(const MinHeap *h, int i)
{
    int l, r, result = -1;

    assert(valid(h, i));

    l = lchild(h, i);
    r = rchild(h, i);
    if (valid(h, l)) {
        result = l;
        if (valid(h, r) && (h->heap[r].prio < h->heap[l].prio)) {
            result = r;
        }
    }
    return result;
}

/* Funzione di supporto: scambia l'elemento in posizione `i` con il
   padre fino a quando raggiunge la posizione corretta nello heap */
static void move_up(MinHeap *h, int i)
{
    int p;

    assert(valid(h, i));

    p = parent(h, i);
    while ( valid(h, p) && (h->heap[i].prio < h->heap[p].prio) ) {
        swap(h, i, p);
        i = p;
        p = parent(h, i);
    }
}

/* Funzione di supporto: scambia l'elemento in posizione `i` con il
   figlio avente priorità minima, fino a quando l'elemento raggiunge
   la posizione corretta. */
static void move_down(MinHeap *h, int i)
{
    int done = 0;

    assert(valid(h, i));

    /* L'operazione viene implementata iterativamente, sebbene sia
       possibile una implementazione ricorsiva probabilmente più
       leggibile. */
    do {
        const int dst = min_child(h, i);
        if (valid(h, dst) && (h->heap[dst].prio < h->heap[i].prio)) {
            swap(h, i, dst);
            i = dst;
        } else {
            done = 1;
        }
    } while (!done);
}

/* Restituisce true (nonzero) se lo heap è vuoto */
int minheap_is_empty(const MinHeap *h)
{
    assert(h != NULL);

    return (h->n == 0);
}

/* Restituisce true (nonzero) se lo heap è pieno, cioè è stata
   esaurita la capienza a disposizione */
int minheap_is_full(const MinHeap *h)
{
    assert(h != NULL);

    return (h->n == h->size);
}

/* Restituisce il numero di elementi presenti nello heap */
int minheap_get_n(const MinHeap *h)
{
    assert(h != NULL);

    return h->n;
}

/* Restituisce la chiave associata alla priorità minima */
int minheap_min(const MinHeap *h)
{
    assert( !minheap_is_empty(h) );

    return h->heap[0].key;
}

/* Come minheap_min(), ma restituisce la coppia (chiave, prio); questa
   funzione verrà utilizzata in future edizioni del corso al posto di
   minheap_min(). */
HeapElem minheap_min2( const MinHeap *h)
{
    assert( !minheap_is_empty(h) );

    return h->heap[0];
}

/* Inserisce una nuova coppia (key, prio) nello heap. */
void minheap_insert(MinHeap *h, int key, double prio)
{
    int i;

    assert( !minheap_is_full(h) );
    assert((key >= 0) && (key < h->size));
    assert(h->pos[key] == -1);

    i = h->n++;
    h->pos[key] = i;
    h->heap[i].key = key;
    h->heap[i].prio = prio;
    move_up(h, i);
}

/* Rimuove la coppia (chiave, priorità) con priorità minima;
   restituisce la chiave associata alla priorità minima. */
int minheap_delete_min(MinHeap *h)
{
    int result;

    assert( !minheap_is_empty(h) );

    result = minheap_min(h);
    swap(h, 0, h->n-1);
    assert( h->heap[h->n - 1].key == result );
    h->pos[result] = -1;
    h->n--;
    if (!minheap_is_empty(h)) {
        move_down(h, 0);
    }
    return result;
}

/* Come minheap_delete_min(), ma restituisce la coppia (chiave, prio);
   questa funzione verrà utilizzata al posto di minheap_delete_min()
   in future edizioni del corso. */
HeapElem minheap_delete_min2(MinHeap *h)
{
    HeapElem result;

    assert( !minheap_is_empty(h) );

    result = minheap_min2(h);
    swap(h, 0, h->n-1);
    assert( h->heap[h->n - 1].key == result.key );
    h->pos[result.key] = -1;
    h->n--;
    if (!minheap_is_empty(h)) {
        move_down(h, 0);
    }
    return result;
}

/* Modifica la priorità associata alla chiave key. La nuova priorità
   può essere maggiore, minore o uguale alla precedente. */
void minheap_change_prio(MinHeap *h, int key, double newprio)
{
    int j;
    double oldprio;

    assert(h != NULL);
    assert(key >= 0 && key < h->size);
    j = h->pos[key];
    assert( valid(h, j) );
    oldprio = h->heap[j].prio;
    h->heap[j].prio = newprio;
    if (newprio > oldprio) {
        move_down(h, j);
    } else {
        move_up(h, j);
    }
}


/*
 * Funzione di supporto 
 * Trasforma la coppia di coordinate ('row', 'col')
 * in un corrispondente indice unico
 */
static int to_vector_idx(int row, int col, int max_cols) 
{
    return row * max_cols + col;
}

/*
 * Wrapper per ottenere un identificativo unico per una cella di una matrice
 */
static int get_unique_id(Coordinates position, int max_cols)
{
    return to_vector_idx(position.row, position.col, max_cols);
}

long **read_matrix_from_file(FILE *fin, int rows, int cols) 
{
    long **out_matrix; /* Output matrix */
    Coordinates position; /* Indici della matrice */
    assert(fin != NULL);

    out_matrix = (long **)malloc(sizeof(long **) * rows);
    for (position.row = 0; position.row < rows; position.row++) 
    {
        out_matrix[position.row] = (long *)malloc(sizeof(long) * cols);
        for (position.col = 0; position.col < cols; position.col++) 
        {
            fscanf(fin, "%ld", &out_matrix[position.row][position.col]);
        }
    }

    return out_matrix;
}

void insert_node(Node **graph, Node *to_insert) 
{
    assert(to_insert != NULL);

    to_insert->next = *graph;
    *graph = to_insert;
}

void insert_edge(Node *nodo, Edge *to_insert, Direction idx) 
{
    assert(nodo != NULL);
    nodo->links[idx] = to_insert;
}

Node *find_node_by_id(Graph *graph, int id) 
{
    Node *current_node;

    for (current_node = graph->nodes; current_node != NULL; current_node = current_node->next) 
    {
        if (current_node->id == id)
            return current_node;
    }

    return NULL;
}

int is_within_boundaries(Coordinates* position, int rows, int cols) 
{
    return 
        (position->row >= 0 && position->row < rows) &&
        (position->col >= 0 && position->col < cols);
}

double get_cost(long c_cell, long c_height, long current_cell, long neighbour)
{
    return c_cell + c_height * pow(current_cell - neighbour, 2);
}

Graph *build_graph(long **values, int rows, int cols, long c_cell, long c_height)
{
    int directions[4][2] = {
        {-1, 0}, /* UP */
        {0, 1}, /* RIGHT */
        {1, 0}, /* DOWN */
        {0, -1}, /* LEFT */
    };

    Direction current_direction;
    Node *nodes = NULL;
    Coordinates position; /* Indici della matrice */
    Coordinates next_position;
    Node *new_node;
    Edge *new_edge;

    Graph *out_graph = (Graph *)malloc(sizeof(Graph));
    assert(out_graph != NULL);

    for (position.row = 0; position.row < rows; position.row++) 
    {
        for (position.col = 0; position.col < cols; position.col++) 
        {
            new_node = (Node *)malloc(sizeof(Node));
            assert(new_node != NULL);
            for (current_direction = UP; current_direction < 4; current_direction++) 
            {
                next_position.row = position.row + directions[current_direction][0];
                next_position.col = position.col + directions[current_direction][1];

                if (!is_within_boundaries(&next_position, rows, cols)) {
                    insert_edge(new_node, NULL, current_direction); 
                    continue; 
                }

                new_edge = (Edge *)malloc(sizeof(Edge));
                assert(new_edge != NULL);

                new_edge->src = get_unique_id(position, cols);
                new_edge->dst = get_unique_id(next_position, cols);
                new_edge->weight = get_cost(c_cell, c_height, values[position.row][position.col], values[next_position.row][next_position.col]);

                insert_edge(new_node, new_edge, current_direction); 
            }
            
            new_node->id = get_unique_id(position, cols);
            new_node->height = values[position.row][position.col];
            new_node->coords = position;
            new_node->parent = NULL;
            new_node->cost_from_src = 0;

            insert_node(&nodes, new_node);
        }
    }

    out_graph->nodes = nodes;
    out_graph->nr_nodes = rows * cols;
    
    return out_graph;
}

void print_graph(Graph* graph) 
{
    Node *current_node;
    Direction idx_direction;
    char *directions_string[] = {"UP", "RIGHT", "DOWN", "LEFT"};

    for (current_node = graph->nodes; current_node != NULL; current_node = current_node->next) 
    {
        printf("Nodo [%d] --- %d mt s.l.m.\n", current_node->id, current_node->height);
        printf("# Parent: [%d]\n", current_node->parent->id);

        for (idx_direction = UP; idx_direction < 4; idx_direction++) 
        {
            if (current_node->links[idx_direction] == NULL)
                printf("\t%s: NULL", directions_string[idx_direction]);
            else
                printf("\t%s: %d - weight = %.2f;", directions_string[idx_direction], current_node->links[idx_direction]->dst, current_node->links[idx_direction]->weight);
            
            puts("");
        }
        puts("");        
    }
}

void print_matrix(long **matrix, int rows, int cols) 
{
    Coordinates position; /* Indici della matrice */

    assert(matrix != NULL);
    for (position.row = 0; position.row < rows; position.row++) 
    {
        assert(matrix[position.row] != NULL);
        for (position.col = 0; position.col < cols; position.col++) 
        {
            printf("%ld ", matrix[position.row][position.col]);
        }

        puts("");
    }
}

void fill_queue(MinHeap *queue, Graph *graph, long c_cell) 
{
    Node *current_node;

    for (current_node = graph->nodes; current_node != NULL; current_node = current_node->next) 
    {
        if (current_node->id == 0) /* se ci troviamo sulla sorgente, il costo sarà c_cell */
            minheap_insert(queue, current_node->id, c_cell);
        else
            minheap_insert(queue, current_node->id, FLT_MAX);
    }
}

double relax(HeapElem *src, HeapElem *dst, Edge *link) 
{
    double new_weight = 0;

    if (dst->prio > src->prio + link->weight) 
    {
        new_weight = src->prio + link->weight;
        dst->prio = new_weight;
    }

    return new_weight;
}

void dijkstra(Graph *graph, long c_cell) 
{
    MinHeap *queue = minheap_create(graph->nr_nodes);
    HeapElem extracted, heap_adj;
    Node *src_node, *dst_node;
    Edge **adj;
    Direction idx_direction;
    double new_prio;
    int j;

    fill_queue(queue, graph, c_cell);

    while ( !minheap_is_empty(queue) ) 
    {
        extracted = minheap_delete_min2(queue);
        
        src_node = find_node_by_id(graph, extracted.key);
        adj = src_node->links;

        for (idx_direction = UP; idx_direction < 4; idx_direction++) 
        {
            if (adj[idx_direction] == NULL) continue;

            j = queue->pos[adj[idx_direction]->dst];
            heap_adj = queue->heap[j];
            
            new_prio = relax(
                &extracted, 
                &heap_adj,
                adj[idx_direction]);

            if (new_prio) 
            {
                minheap_change_prio(queue, adj[idx_direction]->dst, new_prio);
                dst_node = find_node_by_id(graph, adj[idx_direction]->dst);

                dst_node->parent = src_node;
                dst_node->cost_from_src = new_prio;
            }
        }
    }
}

void print_shortest_path(Graph *graph, int src_id, int dst_id) 
{
    Node *src_node = find_node_by_id(graph, src_id);
    Node *dst_node = find_node_by_id(graph, dst_id);
    Node *current_node;

    Node *reversed_list = NULL;
    
    for (current_node = dst_node; current_node->id != src_node->id; current_node = current_node->parent) {
        insert_node(&reversed_list, current_node);
    }

    insert_node(&reversed_list, src_node);

    for (current_node = reversed_list; current_node != NULL; current_node = current_node->next) 
    {
        printf("%d %d\n", current_node->coords.col, current_node->coords.row);
    } 
    printf("-1 -1\n");
    printf("%.0f", dst_node->cost_from_src);
}

int main(int argc, char *argvs[]) 
{
    FILE *input_file;
    int n, m; /* 'n' righe, 'm' colonne della matrice input */
    long c_cell, c_height;
    long **H; /* Matrice di input */
    Coordinates dst_coords;
    Graph *map;
    
    assert(argc > 1);
    
    input_file = fopen(argvs[1], "r");
    assert(input_file != NULL);

    /* Lettura dati input */
    fscanf(input_file, "%ld", &c_cell);
    fscanf(input_file, "%ld", &c_height);
    fscanf(input_file, "%d", &n);
    fscanf(input_file, "%d", &m);

    dst_coords.row = n - 1;
    dst_coords.col = m - 1;

    H = read_matrix_from_file(input_file, n, m);

    map = build_graph(H, n, m, c_cell, c_height);

    dijkstra(map, c_cell);
    print_shortest_path(map, 0, get_unique_id(dst_coords, m));

    fclose(input_file);
    return EXIT_SUCCESS;
}