#include "btreestore.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>


// preorder traverse to print each key in node
void preorder_traverse(tree_node * n) {
    tree_node * curr = n;
    puts("node keys: ");
    for(int j = 0; j < curr->num_elements; j++) {
        printf("%d ", curr->keys[j]);
    }
    puts("");

    int i = 0;
    while(i < curr->num_child) {
        preorder_traverse(curr->children[i]);
        i++;
    }
    return;
}

// insert multiple
int test_insert1(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 1;
    uint32_t nb = 2;

    void * h = init_store(4, 1);
    int suc1 = btree_insert(1, (void *)p, 2, key, n, h);
    int suc2 = btree_insert(1, (void *)p, 2, key, n, h);
    int suc3 = btree_insert(2, (void *)p, 2, key, n, h);
    int suc4 = btree_insert(3, (void *)p, 2, key, n, h);
    int suc5 = btree_insert(4, (void *)p, 2, key, n, h);
    int suc6 = btree_insert(5, (void *)p, 2, key, n, h);
    int suc7 = btree_insert(6, (void *)p, 2, key, n, h);
    
    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);
    close_store(h);
}

// insert split required
int test_insert2(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 1;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    btree_insert(1, (void *)p, 16, key, n, h);
    btree_insert(2, (void *)p, 16, key, n, h);
    btree_insert(3, (void *)p, 16, key, n, h);
    btree_insert(5, (void *)p, 16, key, n, h);
    btree_insert(6, (void *)p, 16, key, n, h);
    btree_insert(8, (void *)p, 16, key, n, h);
    btree_insert(80, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);
    close_store(h);
}

// insert no splite
int test_insert3(){

    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 1;
    uint32_t nb = 2;


    void * h = init_store(3, 1);
    btree_insert(9, (void *)p, 16, key, n, h);
    btree_insert(10, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);
    close_store(h);
}

// insert to empty tree
int test_insert4(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    btree_insert(9, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);
    close_store(h);
}

// insert already exist
int test_insert5(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    hlper * hp = (hlper *)h;

    btree_insert(9, (void *)p, 16, key, n, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    btree_insert(9, (void *)p, 16, key, n, h);
    printf("\ntotal node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);
    close_store(h);
}

// multiple delete
int test_delete1(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    
    btree_insert(1, (void *)p, 16, key, n, h);
    btree_insert(80, (void *)p, 16, key, n, h);
    btree_insert(21, (void *)p, 16, key, n, h);
    btree_insert(22, (void *)p, 16, key, n, h);
    btree_insert(3, (void *)p, 16, key, n, h);
    btree_insert(2, (void *)p, 16, key, n, h);
    btree_insert(6, (void *)p, 16, key, n, h);
    btree_insert(5, (void *)p, 16, key, n, h);
    btree_insert(8, (void *)p, 16, key, n, h);
    btree_insert(20, (void *)p, 16, key, n, h);
    btree_insert(4, (void *)p, 16, key, n, h);
    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(6, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(2, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(21, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(8, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(4, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(22, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    close_store(h);
}

// delete one key root node only
int test_delete2(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    
    btree_insert(1, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(1, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    close_store(h);
}

// delete not exist key
int test_delete3(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    
    btree_insert(1, (void *)p, 16, key, n, h);
    btree_insert(2, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(3, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    close_store(h);
}

// delete leaf, require merge
int test_delete4(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    
    btree_insert(1, (void *)p, 16, key, n, h);
    btree_insert(2, (void *)p, 16, key, n, h);
    btree_insert(3, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(1, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    close_store(h);
}

// delete intennal, borrow required
int test_delete5(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    
    btree_insert(2, (void *)p, 16, key, n, h);
    btree_insert(21, (void *)p, 16, key, n, h);
    btree_insert(20, (void *)p, 16, key, n, h);
    btree_insert(5, (void *)p, 16, key, n, h);
    btree_insert(17, (void *)p, 16, key, n, h);
    btree_insert(11, (void *)p, 16, key, n, h);
    btree_insert(3, (void *)p, 16, key, n, h);
    btree_insert(13, (void *)p, 16, key, n, h);
    btree_insert(19, (void *)p, 16, key, n, h);
    btree_insert(7, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(5, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    close_store(h);
}

// delete internal, merge required
int test_delete6(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    
    btree_insert(2, (void *)p, 16, key, n, h);
    btree_insert(21, (void *)p, 16, key, n, h);
    btree_insert(20, (void *)p, 16, key, n, h);
    btree_insert(5, (void *)p, 16, key, n, h);
    btree_insert(17, (void *)p, 16, key, n, h);
    btree_insert(11, (void *)p, 16, key, n, h);
    btree_insert(3, (void *)p, 16, key, n, h);
    btree_insert(13, (void *)p, 16, key, n, h);
    btree_insert(19, (void *)p, 16, key, n, h);
    btree_insert(7, (void *)p, 16, key, n, h);
    btree_insert(1, (void *)p, 16, key, n, h);
    btree_insert(8, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(8, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    close_store(h);
}

// delete leaf, borrow required
int test_delete7(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    void * h = init_store(3, 1);
    
    btree_insert(2, (void *)p, 16, key, n, h);
    btree_insert(21, (void *)p, 16, key, n, h);
    btree_insert(20, (void *)p, 16, key, n, h);
    btree_insert(5, (void *)p, 16, key, n, h);
    btree_insert(17, (void *)p, 16, key, n, h);
    btree_insert(11, (void *)p, 16, key, n, h);
    btree_insert(3, (void *)p, 16, key, n, h);
    btree_insert(13, (void *)p, 16, key, n, h);
    btree_insert(19, (void *)p, 16, key, n, h);
    btree_insert(7, (void *)p, 16, key, n, h);
    btree_insert(1, (void *)p, 16, key, n, h);
    btree_insert(8, (void *)p, 16, key, n, h);

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    puts("");
    btree_delete(21, h);
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    close_store(h);
}

// mix insert and delete
int test_mix1(){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 1;
    uint32_t nb = 2;

    void * h = init_store(7, 1);
    btree_insert(1, (void *)p, 2, key, n, h);
    btree_insert(20, (void *)p, 2, key, n, h);
    btree_insert(3, (void *)p, 2, key, n, h);
    btree_insert(4, (void *)p, 2, key, n, h);
    btree_insert(40, (void *)p, 2, key, n, h);
    btree_insert(6, (void *)p, 2, key, n, h);
    btree_insert(33, (void *)p, 2, key, n, h);
    btree_insert(8, (void *)p, 2, key, n, h);
    btree_insert(9, (void *)p, 2, key, n, h);
    btree_insert(66, (void *)p, 2, key, n, h);
    btree_insert(11, (void *)p, 2, key, n, h);
    btree_insert(12, (void *)p, 2, key, n, h);

    btree_delete(11, h);
    btree_delete(1, h);
    btree_delete(3, h);

    btree_insert(11, (void *)p, 2, key, n, h);
    btree_insert(19, (void *)p, 2, key, n, h);

    
    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);
    close_store(h);
}


void thread_insert(void * h){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;


    for(int i = 1; i < 50; i++ ){
        btree_insert(i, (void *)p, 16, key, n, h);
    }
}

void thread_op(void * h){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;

    for(int i = 1; i < 100; i++ ){
        btree_insert(i, (void *)p, 16, key, n, h);
    }
    sleep(1);
    for(int i = 1; i < 99; i++ ){
        btree_delete(i,h);
    }
}

void thread_delete(void * h){
    uint32_t key[4] = {1,1,1,1};
    uint64_t p[2] = {1, 2};
    uint64_t c[2] = {0};
    uint64_t n = 7;
    uint32_t nb = 2;


    for(int i = 2; i < 50; i++ ){
        btree_delete(i, h);
    }
}

//delete node by threads, would result in heap mem error if multi thread are delete same key/node
int test_multithread1() {
    void * h = init_store(6, 1);
    thread_insert(h); // insert 1-50 
    int num_thds = 4;
    pthread_t threads[4];
    int i;
    for(i = 0; i < num_thds; i++){
        int * a = malloc(sizeof(int));
        *a = i;
        if(pthread_create(threads + i, NULL, (void *)&thread_delete, h) != 0) {
            puts("fail to create thread");
            free(a);
            return 1;
        }
        free(a);
    }


    for(i = 0; i < num_thds; i++){
        if(pthread_join(threads[i], NULL) != 0) {
            puts("fail to create thread");
            return 1;
        }
    }

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);

    close_store(h);
}

// insert and delete using multi thread
int test_multithread2() {
    void * h = init_store(6, 1);
    int num_thds = 4;
    pthread_t threads[4];
    int i;
    for(i = 0; i < num_thds; i++){
        int * a = malloc(sizeof(int));
        *a = i;
        if(pthread_create(threads + i, NULL, (void *)&thread_op, h) != 0) {
            puts("fail to create thread");
            free(a);
            return 1;
        }
        free(a);
    }


    for(i = 0; i < num_thds; i++){
        if(pthread_join(threads[i], NULL) != 0) {
            puts("fail to create thread");
            return 1;
        }
    }

    hlper * hp = (hlper *)h;
    printf("total node: %ld\n", hp->num_node);
    preorder_traverse(hp->root);
    close_store(h);
}


// encryption, decryption
int test_decrypt1() {
    uint32_t key[4] = {1,1,1,1};
    uint32_t p[2] = {4, 8};
    uint32_t c[2] = {0};
    uint32_t new_p[2] = {0};

    encrypt_tea(p, c, key);
    printf("plain: %d %d\n", p[0], p[1]);

    decrypt_tea(c, new_p, key);
    printf("decryped: %d %d\n", new_p[0], new_p[1]);
}



int main(int argc, char** argv) {
    int (*test_ptrs[])() = {
                test_insert1,
                test_insert2,
                test_insert3,
                test_insert4,
                test_insert5,
                test_delete1,
                test_delete2,
                test_delete3,
                test_delete4,
                test_delete5,
                test_delete6,
                test_delete7,
                test_mix1,
                test_multithread1,
                test_multithread2,
                test_decrypt1,
    };
    
    int suc = (*test_ptrs[atoi(argv[1])])();
}