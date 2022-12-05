#include "btreestore.h"

// return number of 8-byte-block required
uint32_t get_num_block(size_t count) {
    int has_rem = (count % sizeof(uint64_t)) != 0;
    uint32_t num_block = (count / sizeof(uint64_t)) + has_rem;
    return num_block;
}

// initialise a node
tree_node * init_node(hlper * h) {
    tree_node * curr = (tree_node *)malloc(sizeof(tree_node));
    curr->keys = (uint32_t *)malloc(sizeof(uint32_t) * h->branching); // (max is b-1) -> but an extra space for ease of operations
    curr->vals = malloc(sizeof(struct info *) * h->branching);
    curr->num_elements = 0;
    curr->parent = NULL;
    curr->children = (tree_node **)malloc(sizeof(tree_node *) * (h->branching + 1)); // extra space for ease of operations
    curr->num_child = 0;
    h->num_node += 1;
    return curr;
}

// return 0 if key already exist, return 1 if not also gets node where search ends
int search(tree_node * root, uint32_t key, tree_node ** end_node) {
    tree_node * curr = root;
    // while curr is not a leaf node
    while(curr->num_child != 0) {
        // check and decide
        int i = 0;
        while(i < curr->num_elements) {
            // key found
            if(curr->keys[i] == key) {
                *end_node = curr;
                return 0;
            } 
            // go to child i if smaller then current-key
            if(key < curr->keys[i]) {
                curr = curr->children[i];
                break;
            }
            // go to child i+1 if key greater then last key in node and that child exist
            if(i == curr->num_elements - 1 && key > curr->keys[i]) {
                curr = curr->children[i + 1];
                break;
            }
            i++;
        }
    }

    // check on the leaf node
    int j = 0;
    while(j < curr->num_elements) {
        // key found
        if(curr->keys[j] == key) {
            *end_node = curr;
            return 0;
        } 
        j++;
    }
    *end_node = curr;
    return 1;
}

// get index of key in node
int get_key_index(tree_node * n, uint32_t key) {
    int index = -1;
    for(int i = 0; i < n->num_elements; i++){
        if(n->keys[i] == key){
            index = i;
            break;
        }
    }
    return index;
}

// get index of child in node's children arr
int get_child_index(tree_node * parent, tree_node * child) {
    int index = -1;
    for(int i = 0; i < parent->num_child; i++){
        if(parent->children[i] == child){
            index = i;
            break;
        }
    }
    return index;
}

// shift k-v in node from index up to last -> reduce num of elements by 1
void shift_left(tree_node * n, int index) {
    n->num_elements -= 1;
    for(int i = index; i < n->num_elements; i++) {
        n->keys[i] = n->keys[i + 1];
        n->vals[i] = n->vals[i + 1];
    }
}

// shift k-v right by 1, from index down to 0
void shift_right(tree_node * n, int index) {
    for(int i = n->num_elements; i > index; i--) {
        n->keys[i] = n->keys[i - 1];
        n->vals[i] = n->vals[i - 1];
    }
    n->num_elements += 1;
}

// free a node
void free_node(tree_node * n) {
    free((void *)(n->keys));
    free((void *)(n->vals));
    free((void *)(n->children));
    free((void *)(n));
}

// post-order travsersal to free all node
void free_traverse(tree_node * n) {
    tree_node * curr = n;
    int i = 0;
    while(i < curr->num_child) {
        free_traverse(curr->children[i]);
        i++;
    }
    
    for(int j = 0; j < curr->num_elements; j++) {
        void * cipher = ((struct info *)curr->vals[j])->data;
        free(cipher);
        free((void *)(curr->vals[j]));
    }
    free_node(curr);
}

// recursive pre-order traverse for export
void store_preorder(tree_node * n, void ** store_addr_ptr) {
    tree_node * curr = n;
    struct node * store_node = (struct node *)(*store_addr_ptr);
    store_node->num_keys = curr->num_elements;
    uint32_t * k = (uint32_t *)malloc(sizeof(uint32_t) * curr->num_elements);
    memcpy(k, curr->keys, sizeof(uint32_t) * curr->num_elements);
    store_node->keys = k;
    *store_addr_ptr += sizeof(struct node);

    int i = 0;
    while(i < curr->num_child) {
        store_preorder(curr->children[i], store_addr_ptr);
        i++;
    }
}

// merge
void merge(tree_node * parent, int kv_index, tree_node * left_child, tree_node * right_child, hlper * h) {
    // add k-v to left child
    int last_index = left_child->num_elements; // kept track of merged node
    left_child->keys[last_index] = parent->keys[kv_index];
    left_child->vals[last_index] = parent->vals[kv_index];
    left_child->num_elements += 1;
    last_index += 1;

    // shift parent as k-v has been removed
    shift_left(parent, kv_index);

    // merge right node
    for(int i = 0; i < right_child->num_elements; i++) {
        left_child->keys[last_index] = right_child->keys[i];
        left_child->vals[last_index] = right_child->vals[i];
        left_child->num_elements += 1;
        last_index += 1;
    }
    // also right's child
    int last_child = left_child->num_child;
    for(int i = 0; i < right_child->num_child; i++) {
        left_child->children[last_child] = right_child->children[i];
        left_child->children[last_child]->parent = left_child;
        left_child->num_child += 1;
        last_child += 1;
    }
    // free mem allocated for right node
    free_node(right_child);

    // shift parent's child arr
    parent->num_child -= 1;
    for(int i = (kv_index + 1); i < parent->num_child; i++) {
        parent->children[i] = parent->children[i + 1];
    }

    // if parent is empty after merge
    if(parent->num_elements == 0) {
        // if parent is root -> free mem, left child became new root
        if(parent->parent == NULL) {
            free_node(parent);
            h->root = left_child;
            left_child->parent = NULL;
            h->num_node -= 1;
        // grand parent is not NULL, merge parent with sibling(uncle)
        } else {
            tree_node * grand_parent = parent->parent;
            // get index 
            int p_index = get_child_index(grand_parent, parent);
            tree_node * left_parent = NULL;
            tree_node * right_parent = NULL;
            int g_index;
            // if parent not most left child
            if(p_index > 0) {
                left_parent = grand_parent->children[p_index - 1];
                right_parent = parent;
                g_index = p_index - 1;
            } else { // parent if most left
                left_parent = parent;
                right_parent = grand_parent->children[p_index + 1];
                g_index = p_index;
            }

            merge(grand_parent, g_index, left_parent, right_parent, h);
            parent = left_parent;
        }
    }
    // update num_node
    h->num_node -= 1;
}

// recursive delete
void delete(tree_node * subtree_root, uint32_t key, hlper * h) {
    // get expect index
    int kv_index = 0;
    while(kv_index < subtree_root->num_elements) {
        if(subtree_root->keys[kv_index] >= key) {
            break;
        }
        kv_index += 1;
    }

    // if subtree root is leaf -> key must be in it -> simply remove the key from it -> shift k-v arr
    if(subtree_root->num_child == 0 ) {
        if(subtree_root->num_elements >= (int)((h->branching + 1)/2) || subtree_root->parent == NULL) {
            // make shift
            shift_left(subtree_root, kv_index);
            return;
        } else {
            // otherwise need to ensure no underflow when k-v is deleted -> borrow or merge
            // left and right sibling of next_subroot if exist
            tree_node * parent = subtree_root->parent;
            tree_node * left_sibl = NULL; 
            tree_node * right_sibl = NULL; 
            int r_index = get_child_index(parent, subtree_root); // root index
            if(r_index > 0) { // subroot not first child -> exist left sibling
                left_sibl = parent->children[r_index - 1];
            }
            if(r_index != parent->num_child - 1) { // subroot not last child -> exist right sibling
                right_sibl = parent->children[r_index + 1];
            }

            // if left is possible to borrow
            if(left_sibl != NULL && left_sibl->num_elements >= (int)((h->branching + 1)/2)) {
                // shift subroot for k-v from to be moved
                shift_right(subtree_root, 0);
                // move parent k-v
                subtree_root->keys[0] = parent->keys[r_index - 1];
                subtree_root->vals[0] = parent->vals[r_index - 1];
                // move left_sibl's last k-v to parent
                parent->keys[r_index - 1] = left_sibl->keys[left_sibl->num_elements -1];
                parent->vals[r_index - 1] = left_sibl->vals[left_sibl->num_elements -1];
                left_sibl->num_elements -= 1;

            // right sibling is possible to borrow
            } else if(right_sibl != NULL && right_sibl->num_elements >= (int)((h->branching + 1)/2)) {
                // move parent k-v to last
                subtree_root->keys[subtree_root->num_elements] = parent->keys[r_index];
                subtree_root->vals[subtree_root->num_elements] = parent->vals[r_index];
                subtree_root->num_elements += 1;
                // move right_sibl's first k-v to parent
                parent->keys[r_index] = right_sibl->keys[0];
                parent->vals[r_index] = right_sibl->vals[0];
                // shift right sibl k-v arr
                shift_left(right_sibl, 0);             
            
            // merge with left
            } else if (left_sibl != NULL) {
                merge(parent, kv_index-1, left_sibl, subtree_root, h);
                subtree_root = left_sibl;
            // merge with right
            } else {
                merge(parent, kv_index, subtree_root, right_sibl, h);
            }
            delete(subtree_root, key, h);
            return;
        }
        
    } else {
    // if subtree root is internal node 
        // if key is in this node
        if(subtree_root->keys[kv_index] == key) {
            // get left right child
            tree_node * left_child = subtree_root->children[kv_index];
            tree_node * right_child = subtree_root->children[kv_index + 1];
            // 1 check can be replaced by max key of left subtree 
            if(left_child->num_elements >= 1) {
                tree_node * max_left = left_child;
                while(max_left->num_child > 1){
                    max_left = max_left->children[max_left->num_child - 1];
                }
                // copy to subtree root
                subtree_root->keys[kv_index] = max_left->keys[max_left->num_elements - 1];
                subtree_root->vals[kv_index] = max_left->vals[max_left->num_elements - 1];
                // recursive delete copied k-v from subtree rooted at left child
                delete(left_child, subtree_root->keys[kv_index], h);

            } else if (right_child->num_elements >= 1){
            // 2 check can be replaced by min key of right subtree
                // similar opeartion as above
                tree_node * min_right = right_child;
                while(min_right->num_child > 1){
                    min_right = min_right->children[0];
                }
                // copy to subtree root
                subtree_root->keys[kv_index] = min_right->keys[0];
                subtree_root->vals[kv_index] = min_right->vals[0];
                // recursive delete copied k-v from subtree rooted at left child
                delete(right_child, subtree_root->keys[kv_index], h);
            
            } else {
            // 3 otherwise add key to left child and merge left right
                merge(subtree_root, kv_index, left_child, right_child, h);
                // delete key from left child
                delete(left_child, key, h);
            }

        } else {
        // if key not in this node -> must be in subtree rooted at child[kv_index]
            tree_node * next_subroot = subtree_root->children[kv_index]; // next subtree_root to be recursed
            
            // if next subroot has more then b//2  
            if(next_subroot->num_elements >= (int)((h->branching + 1)/2) || next_subroot->num_child > 1) {
                delete(next_subroot, key, h);
                return;
            }

            // otherwise need to ensure no underflow when k-v is deleted -> borrow or merge
            tree_node * left_sibl = NULL; 
            tree_node * right_sibl = NULL; 
            if(kv_index != 0) { // subroot not first child -> exist left sibling
                left_sibl = subtree_root->children[kv_index - 1];
            }
            if(kv_index != subtree_root->num_child - 1) { // subroot not last child -> exist right sibling
                right_sibl = subtree_root->children[kv_index + 1];
            }

            // check if we can borrow k-v from either sibling
            // if left is possible to borrow
            if(left_sibl != NULL && left_sibl->num_elements >= (int)((h->branching + 1)/2)) {
                // shift subroot for k-v from to be moved
                shift_right(next_subroot, 0);
                // move parent k-v
                next_subroot->keys[0] = subtree_root->keys[kv_index - 1];
                next_subroot->vals[0] = subtree_root->vals[kv_index - 1];
                // move left_sibl's last k-v to parent
                subtree_root->keys[kv_index - 1] = left_sibl->keys[left_sibl->num_elements -1];
                subtree_root->vals[kv_index - 1] = left_sibl->vals[left_sibl->num_elements -1];
                left_sibl->num_elements -= 1;

                // move left_sibl's last child to subroot's first child if exist
                if(left_sibl->num_child > 0) {
                    // shift subroot's child arr for it to be moved
                    for(int i = next_subroot->num_child; i > 0; i--) {
                        next_subroot->children[i] = next_subroot->children[i - 1];
                    }
                    // move child
                    next_subroot->children[0] = left_sibl->children[left_sibl->num_child - 1];
                    left_sibl->children[left_sibl->num_child - 1]->parent = next_subroot;
                    left_sibl->num_child -= 1;
                    next_subroot->num_child += 1;
                }

            // right sibling is possible to borrow
            } else if(right_sibl != NULL && right_sibl->num_elements >= (int)((h->branching + 1)/2)) {
                // move parent k-v to last
                next_subroot->keys[next_subroot->num_elements] = subtree_root->keys[kv_index];
                next_subroot->vals[next_subroot->num_elements] = subtree_root->vals[kv_index];
                next_subroot->num_elements += 1;
                // move right_sibl's last k-v to parent
                subtree_root->keys[kv_index] = right_sibl->keys[0];
                subtree_root->vals[kv_index] = right_sibl->vals[0];
                // shift right sibl k-v arr
                shift_left(right_sibl, 0);
                
                // move right's first child to subroot's last child if exist
                if(right_sibl->num_child > 0) {
                    // move child
                    next_subroot->children[next_subroot->num_child] = right_sibl->children[0];
                    right_sibl->children[0]->parent = next_subroot;
                    right_sibl->num_child -= 1;
                    next_subroot->num_child += 1;
                    // shift right's child arr
                    for(int i = 0; i < right_sibl->num_child; i++) {
                        right_sibl->children[i] = right_sibl->children[i + 1];
                    }
                }
            
            // merge with left
            } else if (left_sibl != NULL) {
                merge(subtree_root, kv_index-1, left_sibl, next_subroot, h);
                next_subroot = left_sibl;
            // merge with right
            } else {
                merge(subtree_root, kv_index, next_subroot, right_sibl, h);
            }
            delete(next_subroot, key, h);
            return;
        }
    }
}

// recursive insert
void insert(tree_node * insert_node, uint32_t key, struct info * val, hlper * h){
    // get expect index
    int index = 0;
    while(index < insert_node->num_elements) {
        if(key < insert_node->keys[index]) {
            break;
        }
        index++;
    }

    // check if shift required for insert and shift
    shift_right(insert_node, index);
    // insert to expect index
    insert_node->keys[index] = key;
    insert_node->vals[index] = val;

    // check num_elements does not exceed b-1
    if(insert_node->num_elements < h->branching) {
        // no further operation required
        return;
    }

    // node with element greater then b-1
    // get median
    int m_index = (insert_node->num_elements - 1) / 2;
    uint32_t m_key = insert_node->keys[m_index];
    void * m_val = insert_node->vals[m_index];

    // split current node to left and right
    tree_node * right_child = init_node(h); // init new node for right, left is the original node
    right_child->parent = insert_node->parent; // assign same parent
    // fill right child k-v
    int x = insert_node->num_elements;
    for(int k = 1; k < (x - m_index); k++) {
        right_child->keys[k-1] = insert_node->keys[m_index + k];
        right_child->vals[k-1] = insert_node->vals[m_index + k];
        right_child->num_elements += 1;
        insert_node->num_elements -= 1; // original node become left node, take away the k-v pairs added to right
    }
    insert_node->num_elements -= 1; //  take away median k-v to be propagate to parent

    // also split children of left and right node
    if(insert_node->num_child > m_index) {
        right_child->num_child = insert_node->num_child - (m_index + 1);
        memcpy(right_child->children, &(insert_node->children[m_index + 1]), (insert_node->num_child - (m_index + 1)) * sizeof(tree_node *));
        insert_node->num_child -= right_child->num_child;
    }
    for(int i = 0; i < right_child->num_child; i++) { 
        right_child->children[i]->parent = right_child;
    }
    // add right_child to parent
    tree_node * parent = insert_node->parent;

    // parent of splited node
    // parent = NULL -> reached root -> create new root
    if(parent == NULL) {
        tree_node * new_root = init_node(h);
        // insert median k-v to parent
        new_root->keys[0] = m_key;
        new_root->vals[0] = m_val;
        new_root->num_elements = 1;
        // assign left and right child
        new_root->children[0] = insert_node;
        new_root->children[1] = right_child;
        right_child->parent = new_root;
        insert_node->parent = new_root;
        new_root->num_child = 2;
        h->root = new_root;
        return; // done
    } 

    // parent not NULL -> not root 
    // get index for right child to insert
    int right_index = get_child_index(parent, insert_node) + 1; // index for right child to be insert

    // check if shift required for insert
    for(int i = parent->num_child; i > right_index; i--) {
        parent->children[i] = parent->children[i - 1];
    }
    // insert to expect index
    parent->children[right_index] = right_child;
    parent->num_child += 1;
    
    // insert median to parent element
    insert(parent, m_key, m_val, h);
    return;
}


// functions to implement
void * init_store(uint16_t branching, uint8_t n_processors) {
    struct hlper * helper = malloc(sizeof(hlper));
    helper->branching = branching;
    helper->n_processors = n_processors;
    helper->root = init_node(helper);
    helper->num_node = 1;
    pthread_mutex_init(&(helper->lock), NULL); // initialise lock
    return (void *) helper;
}

void close_store(void * helper) {
    hlper * h = (hlper *)helper;
    // free all node
    free_traverse(h->root);
    // destroy mutex
    pthread_mutex_destroy(&(h->lock));
    // free helper
    free(helper);
    return;
}

int btree_insert(uint32_t key, void * plaintext, size_t count, uint32_t encryption_key[4], uint64_t nonce, void * helper) {
    if(helper == NULL) {
        return -1;
    }
    hlper * h = (hlper *)helper;
    pthread_mutex_lock(&h->lock);

    tree_node * insert_node;
    int key_exist = search(h->root, key, &insert_node);

    // if key already exist
    if(key_exist == 0) {
        pthread_mutex_unlock(&h->lock);
        return 1;
    }

    // ciphertext
    uint32_t num_block = get_num_block(count);
    void * ciphertext = calloc(num_block, sizeof(uint64_t));
    void * p = calloc(num_block, sizeof(uint64_t));
    memcpy(p, plaintext, count);
    encrypt_tea_ctr(p, encryption_key, nonce, ciphertext, num_block);
    ciphertext = realloc(ciphertext, count);
    free(p);

    // val -> info
    struct info * val = (struct info *)malloc(sizeof(struct info));
    memcpy((void *)val->key, (void *)encryption_key, 4 * sizeof(uint32_t));
    val->nonce = nonce;
    val->size = count;
    val->data = ciphertext;

    insert(insert_node, key, val, h);
    pthread_mutex_unlock(&h->lock);
    return 0;
}

int btree_retrieve(uint32_t key, struct info * found, void * helper) {
    if(helper == NULL) {
        return 1;
    }
    hlper * h = (hlper *)helper;
    pthread_mutex_lock(&h->lock);

    tree_node * key_node;
    int key_exist = search(h->root, key, &key_node);
    if(key_exist == 1){
        pthread_mutex_unlock(&h->lock);
        return 1;
    }

    int i = 0;
    while(i < key_node->num_elements){
        if(key_node->keys[i] == key) {
            struct info * val = key_node->vals[i];
            found->data = val->data;
            memcpy(found->key, val->key, 4*sizeof(uint32_t));
            found->nonce = val->nonce;
            found->size = val->size;
            pthread_mutex_unlock(&h->lock);
            return 0;
        }
        i++;
    }
    pthread_mutex_unlock(&h->lock);
    return 1;
}

int btree_decrypt(uint32_t key, void * output, void * helper) {
    if(helper == NULL) {
        return 1;
    }    

    struct info val;
    int is_found = btree_retrieve(key, &val, helper);
    if(is_found != 0){
        return 1;
    }

    int num_block = get_num_block(val.size);
    void * plain = calloc(num_block, sizeof(uint64_t));
    void * cipher = calloc(num_block, sizeof(uint64_t));
    memcpy(cipher, val.data, val.size);
    decrypt_tea_ctr(cipher, val.key, val.nonce, plain, num_block);
    plain = realloc(plain, val.size);
    memcpy(output, plain, val.size);
    free(plain);
    free(cipher);
    return 0;
}

int btree_delete(uint32_t key, void * helper) {
    if(helper == NULL) {
        return 0;
    }
    hlper * h = (hlper *)helper;
    pthread_mutex_lock(&h->lock);
    tree_node * aim_node;

    // if key does not exist
    int key_exist = search(h->root, key, &aim_node);
    if(key_exist == 1) {
        pthread_mutex_unlock(&h->lock);
        return 1;
    }
    // get index of key in node;
    int index = get_key_index(aim_node, key);
    // free allocated mem for val
    void * cipher = ((struct info *)aim_node->vals[index])->data;
    free(cipher);
    free((void *)(aim_node->vals[index]));

    delete(h->root, key, h);
    pthread_mutex_unlock(&h->lock);
    return 0;
}

uint64_t btree_export(void * helper, struct node ** list) {
    if(helper == NULL || list == NULL){
        return -1;
    }
    hlper * h = (hlper *)helper;
    pthread_mutex_lock(&h->lock);
    if(h->root->num_elements == 0){
        *list = NULL;
        pthread_mutex_unlock(&h->lock);
        return 0;
    }
    struct node * list_ptr = malloc(sizeof(struct node) * h->num_node); // malloc storing array
    *list = list_ptr;
    store_preorder(h->root, (void*) &list_ptr);
    pthread_mutex_unlock(&h->lock);
    return h->num_node;
}

void encrypt_tea(uint32_t plain[2], uint32_t cipher[2], uint32_t key[4]) {
    // Your code here
    uint32_t sum = 0;
    uint32_t delta = 0x9E3779B9;
    cipher[0] = plain[0];
    cipher[1] = plain[1];
    for(int i = 0; i < 1024; i++) {
        sum = (sum + delta) % TWO_TO_THIRTYTWO;
        uint32_t tmp1 = ((cipher[1] << 4) + key[0]) % TWO_TO_THIRTYTWO;
        uint32_t tmp2 = (cipher[1] + sum) % TWO_TO_THIRTYTWO;
        uint32_t tmp3 = ((cipher[1] >> 5) + key[1]) % TWO_TO_THIRTYTWO;
        cipher[0] = (cipher[0] + (tmp1 ^ tmp2 ^ tmp3)) % TWO_TO_THIRTYTWO;
        uint32_t tmp4 = ((cipher[0] << 4) + key[2]) % TWO_TO_THIRTYTWO;
        uint32_t tmp5 = (cipher[0] + sum) % TWO_TO_THIRTYTWO;
        uint32_t tmp6 = ((cipher[0] >> 5) + key[3]) % TWO_TO_THIRTYTWO;
        cipher[1] = (cipher[1] + (tmp4 ^ tmp5 ^ tmp6)) % TWO_TO_THIRTYTWO;
    }
    return;
}

void decrypt_tea(uint32_t cipher[2], uint32_t plain[2], uint32_t key[4]) {
    uint32_t sum = 0xDDE6E400;
    uint32_t delta = 0x9E3779B9;
    for(int i = 0; i < 1024; i++) {
        uint32_t tmp4 = ((cipher[0] << 4) + key[2]) % TWO_TO_THIRTYTWO;
        uint32_t tmp5 = (cipher[0] + sum) % TWO_TO_THIRTYTWO;
        uint32_t tmp6 = ((cipher[0] >> 5) + key[3]) % TWO_TO_THIRTYTWO;
        cipher[1] = (cipher[1] - (tmp4 ^ tmp5 ^ tmp6)) % TWO_TO_THIRTYTWO;
        uint32_t tmp1 = ((cipher[1] << 4) + key[0]) % TWO_TO_THIRTYTWO;
        uint32_t tmp2 = (cipher[1] + sum) % TWO_TO_THIRTYTWO;
        uint32_t tmp3= ((cipher[1] >> 5) + key[1]) % TWO_TO_THIRTYTWO;
        cipher[0] = (cipher[0] - (tmp1 ^ tmp2 ^ tmp3)) % TWO_TO_THIRTYTWO;
        sum = (sum - delta) % TWO_TO_THIRTYTWO;
    }
    plain[0] = cipher[0];
    plain[1] = cipher[1];
    return;
}

void encrypt_tea_ctr(uint64_t * plain, uint32_t key[4], uint64_t nonce, uint64_t * cipher, uint32_t num_blocks) {
    for(int i = 0; i < num_blocks; i++) {
        uint64_t tmp1 = i ^ nonce;
        uint64_t tmp2 = 0;
        encrypt_tea((uint32_t *)&tmp1, (uint32_t *)&tmp2, key);
        cipher[i] = plain[i] ^ tmp2;
    }
    return;
}

void decrypt_tea_ctr(uint64_t * cipher, uint32_t key[4], uint64_t nonce, uint64_t * plain, uint32_t num_blocks) {
    for(int i = 0; i < num_blocks; i++) {
        uint64_t tmp1 = i ^ nonce;
        uint64_t tmp2 = 0;
        encrypt_tea((uint32_t *)&tmp1, (uint32_t *)&tmp2, key);
        plain[i] = cipher[i] ^ tmp2;
    }
    return;
}
