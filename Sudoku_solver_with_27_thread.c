#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 

#define thread_number 27

typedef struct
{
    int row;
    int col;
    int (*data_list)[9];

} thread_param;

int thread_end = 0;
int solver = 0;

int Create_Thread(int (*sudoku)[9]);
void *look_matrix(void *params);
void *look_row(void *params);
void *look_column(void *params);

int number_of_thread = 0;
pthread_mutex_t lock;

        /*  Örnek sudoku
            {6, 2, 4, 5, 3, 9, 1, 8, 7},
            {5, 1, 9, 7, 2, 8, 6, 3, 4},
            {8, 3, 7, 6, 1, 4, 2, 9, 5},
            {1, 4, 3, 8, 6, 5, 7, 2, 9},
            {9, 5, 8, 2, 4, 7, 3, 6, 1},
            {7, 6, 2, 3, 9, 1, 4, 5, 8},
            {3, 7, 1, 9, 5, 6, 8, 4, 2},
            {4, 9, 6, 1, 8, 2, 5, 7, 3},
            {2, 8, 5, 4, 7, 3, 9, 1, 6} */

int main(void)
{
    // sudoku tanımı yapılan yer.
    
    int value = 0;
    int sudoku[9][9] = {0};
    for (int i = 0; i < 9; i++){
      for (int j = 0; j < 9; j++){
      printf("Sudoku elemanı giriniz: ");
      scanf("%d", &value);
      sudoku[i][j] = value;
      }
    }

    
    // mutex kilidi ayarları yapılır. C kendisinde linuxda çalışan mutex kilitlerine sahiptir.
    // threadler fonksiyonumuzu çalıştırıp threadlerimizi oluşturuyoruz.
    if (pthread_mutex_init(&lock, NULL) != 0) { 
      printf("\n Mutex oluşturulamadı.. tekrar deneyin.\n"); 
      return -1; 
    } 
    else if(Create_Thread(sudoku) == -1){
      printf("\n Threadler oluşturulamadı.. tekrar deneyin.\n");
      return -1;
    }
    // Threadler bitene kadar Ana threadimiz diğer threadlerin bitmesini bekler.
    while(thread_end != 27){
      printf("sudoku çözümleniyor...\n");
      sleep(1/20);
    }
    // 27 olması durumunda sudoku doğru çözümlenmiştir.
    printf("%d\n",solver);


    if(solver == 27){
      printf("sudoku doğru\n");
    }
    else{
      printf("sudoku yanlış\n");
    }
    return 0;
    
  
}


int Create_Thread(int (*sudoku)[9]){
  // 27 adet threadimizi bir threadler serisi listesinde tutuyoruz.
  // sonrasında sudokudan teker teker ilgili başlangıç değerlerinden threadlere doğru değerler yollanır.
  pthread_t threads[thread_number];
  for (int i = 0; i < 9; i++)
  {
      for (int j = 0; j < 9; j++)
      {
          // 3x3 matris burada kontrol edilir.
          if (i%3 == 0 && j%3 == 0)
          {
              thread_param* matrix = (thread_param*)calloc(1,sizeof(thread_param));
              matrix->row = i;
              matrix->col = j;
              matrix->data_list = sudoku;
              pthread_create(&threads[number_of_thread++], NULL, look_matrix, matrix);
          }

          // satırlar burada kontrol edilir.
          if (j == 0)
          {
              thread_param* row = (thread_param*)calloc(1,sizeof(thread_param));
              row->row = i;            
              row->data_list = sudoku;
              pthread_create(&threads[number_of_thread++], NULL, look_row, row);
          }

          // sütunlar burada kontrol edilir.
          if (i == 0)
          {
              thread_param* column = (thread_param*)calloc(1,sizeof(thread_param));
              column->col = j;
              column->data_list = sudoku;
              pthread_create(&threads[number_of_thread++], NULL, look_column, column);
          }
      }
  }
  //Threadlerin oluşturulup oluşturulmaması kontrol edilir.
  if (number_of_thread == 27){
    return 0;
  }
  else{
    return -1;
  }
}

void *look_matrix(void *matrix)
{
    // Struct içinde threade yolladığımız değerler void olarak yollanır, bunlar tekrardan struct tipine cast edilir. matrisin satır ve sütunlarının başladığı değerler struct içinden elde edilir. bir array'e aktarılarak burada tekrar eden sayı var mı kontrolü yapılır.
    thread_param* data = (thread_param*) matrix;
    int start_row = data->row;
    int start_column = data->col;
    int dataArray[9] = {0};
    int count = 0;
    int decision = 0;
    
    for (int i = start_row; i < start_row + 3; ++i)
    {
        for (int j = start_column; j < start_column + 3; ++j)
        {
            int val = data->data_list[i][j];
            dataArray[count] = val;
            count++;
  
        }
    }
    for (int k = 0;k<9;k++){
      for(int m = k+1; m<9;m++){
        if (dataArray[k] == dataArray[m]){
          decision = 0;
          break;
        }
        else{
          decision = 1;
        }
      }
      if (decision == 0){
          break;
        }
    }

    // kritik bölgede sudoku çözücü mutex kilidi içinde önce sudoku çözücü değişkenini sonrasında ise thread'in bittiğini belirten thread_end değişkeni gerekli arttırmalara maruz kalır.
    pthread_mutex_lock(&lock);
    if (decision == 1){
      solver++;
    }
    thread_end++;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}



void *look_row(void *row)
{
    // Struct içinde threade yolladığımız değerler void olarak yollanır, bunlar tekrardan struct tipine cast edilir. İçeriden sudoku matrisi ve satırların değeri elde edilir. Bu şekilde 9 satır tek tek arraylere eklenir. her thread tekrar eden sayı var mı diye kontrol eder.
    thread_param* data = (thread_param*) row;
    int start_row = data->row;
    int dataArray[9] = {0};
    int count = 0;
    int decision = 0;

    for (int j = 0; j < 9; j++)
    {
        int val = data->data_list[start_row][j];
        dataArray[count] = val;
        count++;
     
    }

    for (int k = 0;k<9;k++){
      for(int m = k+1; m<9;m++){
        if (dataArray[k] == dataArray[m]){
          decision = 0;
          break;
        }
        else{
          decision = 1;
        }
      }
      if (decision == 0){
          break;
        }
    }
    // kritik bölgede sudoku çözücü mutex kilidi içinde önce sudoku çözücü değişkenini sonrasında ise thread'in bittiğini belirten thread_end değişkeni gerekli arttırmalara maruz kalır.
    pthread_mutex_lock(&lock);
    if(decision == 1){
      solver++;
    }
    thread_end++;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}


void *look_column(void *column)
{
    // Struct içinde threade yolladığımız değerler void olarak yollanır, bunlar tekrardan struct tipine cast edilir. İçeriden sudoku matrisi ve sütunların değeri elde edilir. Bu şekilde 9 sütun tek tek arraylere eklenir. her thread tekrar eden sayı var mı diye kontrol eder.
    thread_param *data = (thread_param *) column;
    int start_column = data->col;
    int dataArray[9] = {0};
    int count = 0;
    int decision = 0;
    
    for (int i = 0; i < 9; i++)
    {
        int val = data->data_list[i][start_column];
        dataArray[count] = val;
        count++;
    }
  
    for (int k = 0;k<9;k++){
      for(int m = k+1; m<9;m++){
        if (dataArray[k] == dataArray[m]){
          decision = 0;
          break;
        }
        else{
          decision = 1;
        }
      }
      if (decision == 0){
          break;
        }
    }
    // kritik bölgede sudoku çözücü mutex kilidi içinde önce sudoku çözücü değişkenini sonrasında ise thread'in bittiğini belirten thread_end değişkeni gerekli arttırmalara maruz kalır.
    pthread_mutex_lock(&lock);
    if(decision == 1){
      solver++;
    }
    thread_end++;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
 }




