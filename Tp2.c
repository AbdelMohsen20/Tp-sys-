#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int K, T, D;
int *stock;
int completed_sales = 0;
int rejected_sales = 0;
pthread_mutex_t mutex;

// دالة خيوط البيع - النسخة الساذجة
void* sell_naive(void* arg) {
    int id = *(int*)arg;
    time_t start = time(NULL);
    
    while(time(NULL) - start < D) {
        int product = rand() % K;
        int quantity = (rand() % 3) + 1;
        
        if(stock[product] >= quantity) {
            usleep(1000);
            stock[product] -= quantity;
            completed_sales++;
            printf("بيع %d: بيع %d من المنتج %d\n", id, quantity, product);
        } else {
            rejected_sales++;
            printf("بيع %d: رفض - لا يوجد مخزون من المنتج %d\n", id, product);
        }
        usleep(100000);
    }
    return NULL;
}

// دالة خيوط البيع - النسخة المحمية
void* sell_protected(void* arg) {
    int id = *(int*)arg;
    time_t start = time(NULL);
    
    while(time(NULL) - start < D) {
        int product = rand() % K;
        int quantity = (rand() % 3) + 1;
        
        pthread_mutex_lock(&mutex);
        if(stock[product] >= quantity) {
            usleep(1000);
            stock[product] -= quantity;
            completed_sales++;
            printf("بيع %d: بيع %d من المنتج %d\n", id, quantity, product);
        } else {
            rejected_sales++;
            printf("بيع %d: رفض - لا يوجد مخزون من المنتج %d\n", id, product);
        }
        pthread_mutex_unlock(&mutex);
        usleep(100000);
    }
    return NULL;
}

// دالة إعادة التخزين - النسخة الساذجة
void* restock_naive(void* arg) {
    time_t start = time(NULL);
    
    while(time(NULL) - start < D) {
        int product = rand() % K;
        int quantity = (rand() % 6) + 5;
        
        stock[product] += quantity;
        printf("مخزون: أضيف %d إلى المنتج %d\n", quantity, product);
        usleep(200000);
    }
    return NULL;
}

// دالة إعادة التخزين - النسخة المحمية
void* restock_protected(void* arg) {
    time_t start = time(NULL);
    
    while(time(NULL) - start < D) {
        int product = rand() % K;
        int quantity = (rand() % 6) + 5;
        
        pthread_mutex_lock(&mutex);
        stock[product] += quantity;
        pthread_mutex_unlock(&mutex);
        
        printf("مخزون: أضيف %d إلى المنتج %d\n", quantity, product);
        usleep(200000);
    }
    return NULL;
}

void run_simulation(int use_mutex) {
    printf("\n=== %s ===\n", use_mutex ? "النسخة المحمية" : "النسخة الساذجة");
    
    // تهيئة المخزون
    stock = malloc(K * sizeof(int));
    for(int i = 0; i < K; i++) {
        stock[i] = 10;
    }
    completed_sales = 0;
    rejected_sales = 0;
    
    if(use_mutex) {
        pthread_mutex_init(&mutex, NULL);
    }
    
    pthread_t *cashiers = malloc(T * sizeof(pthread_t));
    pthread_t restocker;
    int *ids = malloc(T * sizeof(int));
    
    // إنشاء خيوط البيع
    for(int i = 0; i < T; i++) {
        ids[i] = i;
        if(use_mutex) {
            pthread_create(&cashiers[i], NULL, sell_protected, &ids[i]);
        } else {
            pthread_create(&cashiers[i], NULL, sell_naive, &ids[i]);
        }
    }
    
    // إنشاء خيط إعادة التخزين
    if(use_mutex) {
        pthread_create(&restocker, NULL, restock_protected, NULL);
    } else {
        pthread_create(&restocker, NULL, restock_naive, NULL);
    }
    
    // الانتظار
    sleep(D);
    
    // جمع النتائج
    for(int i = 0; i < T; i++) {
        pthread_join(cashiers[i], NULL);
    }
    pthread_join(restocker, NULL);
    
    // عرض النتائج
    printf("\nالنتائج:\n");
    printf("المخزون النهائي:\n");
    int negative_count = 0;
    for(int i = 0; i < K; i++) {
        printf("المنتج %d: %d", i, stock[i]);
        if(stock[i] < 0) {
            printf(" (سلبي!)");
            negative_count++;
        }
        printf("\n");
    }
    printf("المبيعات المكتملة: %d\n", completed_sales);
    printf("المبيعات المرفوضة: %d\n", rejected_sales);
    printf("المنتجات ذات المخزون السلبي: %d\n", negative_count);
    
    // تنظيف
    free(stock);
    free(cashiers);
    free(ids);
    if(use_mutex) {
        pthread_mutex_destroy(&mutex);
    }
}

int main() {
    srand(time(NULL));
    
    printf("برنامج محاكاة المتجر المصغر\n");
    printf("أدخل عدد المنتجات (K): ");
    scanf("%d", &K);
    printf("أدخل عدد خيوط البيع (T): ");
    scanf("%d", &T);
    printf("أدعد مدة التشغيل (D): ");
    scanf("%d", &D);
    
    // تشغيل النسخة الساذجة
    run_simulation(0);
    
    // تشغيل النسخة المحمية
    run_simulation(1);
    
    printf("\n--- نهاية المحاكاة ---\n");
    return 0;
}
