# Binary Semaphores

## [Inter-task communication and synchronisation]

Binary Semaphores được dùng cho cả mục đích loại trừ lẫn nhau và đồng bộ hóa.

Binary Semarphore và Mutexes rất giống nhau nhứng có một số khác biệt: Mutexes có cơ chế kế thừa ưu tiên, Binary Semaphores thì không. Vì vậy Semaphores là một lựa chọn tốt để thực hiện đồng bộ hóa giữa các Task với nhau, hoặc giữa Task và ISR. Mutexes sẽ phù hợp với việc loại trừ lẫn nhau.

Semaphores API function cho phép chỉ định thời gian mà Blocked state. Block time cho biết sô Ticks tối đa mà Task sẽ chuyển sang Blocked state khi đang cố thực hiện Take Semaphore, nếu Semarphore không có sẵn ngay lập tức. Nếu nhiều Task cùng bị Blocked bởi cùng một Semarphore thì Task có mức ưu tiên cao nhất sẽ được switched out khỏi Blocked state vào lần kế tiếp khi Semarphore available.

Semarphore giống như một Queue chỉ có một phần tử, chỉ có hai trạng thái (Full hoặc Empty). Các Task và ISR không quan tâm Queue đang chứa gì, chỉ cần biết Queue is Full or Empty?.

Hãy xét đến một Task đang thực hiện một tác vụ của ngoại vi. Việc Polling Task gây lãng phí tài nguyên cho CPU và ngăn các Task khác thực thi. Do đó các Task nên dành phần lớn thời gian ở Blocked state (cho các Task khác thực thi, IdleTask chẳng hạn) và chỉ switched in Running state khi có một event nào đó xảy ra. Điều này có thể thực hiện được bằng Binary Semarphore, Block một Task trong khi đang đợi để lấy(`takes`) Semarphore. Sau đó dùng ISR để `gives` Semarphore. Task sẽ luôn thực hiện `takes` từ Semarphore (đọc từ Queue để cho Queue Empty), còn ISR thì luôn `gives` Semarphore (đẩy vào Queue để cho Queue Full).

![Alt text](https://www.freertos.org/fr-content-src/uploads/2018/07/binary-semaphore.gif)

## Function of FreeRTOS Binary Semarphore

#### **`SemaphoreHandle_t`**                            

```C
typedef QueueHandle_t SemaphoreHandle_t;
```

#### **`xSemaphoreCreateBinary`**

> Tạo một Binary Semarphore và return một SemaphoreHandle_t được tham chiếu tới.
> Mỗi Binary Semarphore cần một dung lượng Ram nhỏ để giữ được trạng thái của nó, nếu được khởi tạo bằng hàm `xSemaphoreCreateBinary` thì vùng nhớ sẽ được cấp phát tự động trong FreeRTOS Heap.
> Semaphore được tạo ra sẽ ở trạng thái Empty.

```C
SemaphoreHandle_t xSemaphoreCreateBinary( void );
```

*Return:*

- `NULL`: Semaphore không được khởi tạo vì không đủ FreeRTOS Heap sẵn có.
- Any other value: Semaphore được khởi tạo thành công, Giá trị return sẽ là Handle mà Semaphore được tham chiếu.

*Example:*

```C
SemaphoreHandle_t xSemaphore;

void vATask( void * pvParameters )
{
    /* Attempt to create a semaphore. */
    xSemaphore = xSemaphoreCreateBinary();

    if( xSemaphore == NULL )
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the xSemahore variable.  
        Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
    }
}
```

#### **`vSemaphoreDelete`**

> Xóa một Semaphore, bao gồm cả Semaphore loại Mutex và Semaphore đệ quy.
> Không xóa Semaphore có Task đang Blocked state (Task ở Blocked State chờ semaphore available).

```C
void vSemaphoreDelete( SemaphoreHandle_t xSemaphore );
```

*Parameter:*

- **`xSemaphore`**: Handle của Semaphore muốn xóa.

#### **`xSemaphoreTake`**

> Macro để có được một semaphore. Semaphore phải được khởi tạo trước đó.
> Macro này không nên được gọi trong ISR, thay vào đó có thể gọi `xQueueReceiveFromISR()`

```C
 xSemaphoreTake(    SemaphoreHandle_t xSemaphore,
                    TickType_t xTicksToWait     );
```

*Parameter:*

- **`xSemaphore`**: Handle của Semaphore.
- **`xTicksToWait`**: Thời gian đợi (được tính theo Ticks) cho đến khi Semaphore available. Cần đến macro portTICK_PERIOD_MS có thể được sử dụng để đổi Tick sang milisecond. Blocked Time là `0` tương tự như poll Semaphore. Nếu block time là portMAX_DELAY sẽ cho phép Task block vô hạn.

*Return:*

- `pdTRUE`: nếu semaphore được lấy.
- `pdFALSE`: nếu đã đợi hết xTickToWait mà vẫn chưa nhận được semaphore.

#### **`xSemaphoreTakeFromISR`**

> Một version của `xSemaphoreTake()`, an toàn khi sử dụng trong ngắt, không cần timeout.

```C
xSemaphoreTakeFromISR(  SemaphoreHandle_t xSemaphore,
                        signed BaseType_t *pxHigherPriorityTaskWoken    );
```

*Parameter:*

- **`xSemaphore`**: Handle của Semaphore.
- **`pxHigherPriorityTaskWoken`**: Việc gọi `xSemaphoreTakeFromISR` làm cho một Task đang Bloked switched in Running state sẽ làm cho `*pxHigherPriorityTaskWoken` sẽ được set thành `pdTRUE`.
*Return:*

- `pdTRUE`: nếu semaphore được lấy.

#### **`xSemaphoreGive`**

> Macro để cấp phát một semaphore. Semaphore phải được tạo từ trước.
> Macro này không nên được sử dụng với các Semaphore được tạo bởi `xSemaphoreCreateRecursiveMutex()`.

```C
xSemaphoreGive( SemaphoreHandle_t xSemaphore );
```

*Parameter:*

- **`xSemaphore`**: Handle của Semaphore.
- 
*Return:*

- `pdTRUE`: nếu semaphore được gửi đi thành công.
- `pdFALSE`: nếu semaphore không được gửi đi. Giống với Queue, lỗi này sinh ra khi có một semaphore khác chưa taken.

#### **`xSemaphoreGiveFromISR`**

> Macro để cấp phát một semaphore. Semaphore phải được tạo từ trước. Hàm này an toàn khi sử dụng trong ngắt.
> Macro này không nên được sử dụng với các Semaphore được tạo bởi `xSemaphoreCreateMutex()`.

```C
xSemaphoreGiveFromISR(  SemaphoreHandle_t xSemaphore,
                        signed BaseType_t *pxHigherPriorityTaskWoken    );
```

*Parameter:*

- **`xSemaphore`**: Handle của Semaphore.
- **`pxHigherPriorityTaskWoken`**: Việc gọi `xSemaphoreGiveFromISR` làm cho một Task đang Bloked switched in Running state sẽ `*pxHigherPriorityTaskWoken` sẽ được set thành `pdTRUE`.

*Return:*

- `pdTRUE`: nếu semaphore được gửi đi thành công.
- `pdFALSE`: nếu semaphore không được gửi đi. Giống với Queue, lỗi này sinh ra khi có một semaphore khác chưa taken.

# Counting Semaphores

Binary Semaphore là một Queue có độ dài là 1, Count Semaphore có thể xem là một Queue có độ dài lớn hơn 1. 
Tương tự như Binary Semaphore, ta không cần quan tâm đến dữ liệu được lưu trong Queue, chỉ cần biết được Queue có trống hay không.

Ta thường sử dụng Counting Semaphore với 2 trường hợp:

- Counting Event - Đếm sự kiện
  - Một event handler sẽ gives một semaphore mỗi khi có một sự kiện xảy ra(tăng giá trị count của semaphore) và một Task sẽ takes một semaphore mỗi khi xử lý một event (giảm giá trị count của semaphore). Do đó, giá trị count là sự khác biệt giữa số sự kiện đã xảy ra và số sự kiện đã được xử lý. Trong trường hợp này, lý tưởng là giá trị đếm bằng 0 khi semaphore được tạo.
- Resource management - Quản lý tài nguyên
  - Giá trị count cho biết số lượng tài nguyên có sẵn. Để có được quyền kiểm soát tài nguyên, trước tiên một Task phải có được một semaphore - giảm đến giá trị count semaphore.
  - Khi count về 0 thì sẽ không còn free resource. Trong trường hợp này, lý tưởng sẽ là giá trị đếm bằng với giá trị tối đa khi semaphore được tạo.

## Function of FreeRTOS Counting Semaphore

#### **`xSemaphoreCreateCounting`**

> Tạo một Counting Semarphore và return một SemaphoreHandle_t được tham chiếu tới.
> Mỗi Counting Semarphore cần một dung lượng Ram nhỏ để giữ được trạng thái của nó, nếu được khởi tạo bằng hàm `xSemaphoreCreateCounting` thì vùng nhớ sẽ được cấp phát tự động trong FreeRTOS Heap.
> Semaphore được tạo ra sẽ ở trạng thái Empty.

```C
SemaphoreHandle_t xSemaphoreCreateCounting( UBaseType_t uxMaxCount,
                                            UBaseType_t uxInitialCount  );
```

*Parameter:*

- **`uxMaxCount`**: Giá trị count tối đa là semaphore có thể chứa. Khi semaphore đạt đến giá trị này thì giá trị không thể given được nữa.
- **`uxInitialCount`**: Giá trị đếm được gán cho Semaphore khi nó được khởi tạo.

*Return:*

- Nếu một Counting Semaphore được tạo thành công thì sẽ trả về một Handle được tham chiếu tới.
- Nếu tạo không thành công thì return `NULL`.

*Example:*

```C
void vATask( void * pvParameters )
{
SemaphoreHandle_t xSemaphore;

    /* Create a counting semaphore that has a maximum count of 10 and an initial count of 0. */
    xSemaphore = xSemaphoreCreateCounting( 10, 0 );

    if( xSemaphore != NULL )
    {
        /* The semaphore was created successfully. */
    }
}
```

#### **`uxSemaphoreGetCount`**

> Trả về giá trị Count của Semaphore.

```C
UBaseType_t uxSemaphoreGetCount( SemaphoreHandle_t xSemaphore );
```

*Parameter:*

- **`xSemaphore`**: Handle của Counting Semaphore

*Return:*

- Nếu `xSemaphore` là một Counting Semaphore thì giá trị count hiện tại sẽ được trả về.
- Nếu `xSemaphore` là một Binary Semaphore thì sẽ return 1 nếu semaphore available, return 0 nếu semaphore not available.

*Example:*

```C
void vATask( void * pvParameters )
{
SemaphoreHandle_t xSemaphore;

    /* Create a counting semaphore that has a maximum count of 10 and an initial count of 0. */
    xSemaphore = xSemaphoreCreateCounting( 10, 0 );

    if( xSemaphore != NULL )
    {
        /* The semaphore was created successfully. */
    }
}
```

# Mutexes

Mutexes là Binary Semaphores có thêm cơ chế kế thừa mức ưu tiên. Trong khi Binary Semaphores dùng để thực hiện đồng bộ hóa (giữa Task và ISR), thì Mutexes là một lựa chọn tốt hơn để thực hiện việc loại trừ lẫn nhau ('MUT'ual'EX').

Sử dụng Mutual Exclusion, Mutex hoạt động giống như một token dùng để bảo vệ tài nguyên. Khi một Task muốn truy cập vào tài nguyên, trước tiên phải lấy được token. Khi hoàn thành với tài nguyên, phải trả lại token - cho phép các Task khác có cơ hội truy cập vào cùng tài nguyên.

Mutexes cũng sử dụng API của Semaphore, do đó cũng có block time. Block time là số Ticks tối đa mà để một Task bị đưa vào Blocked State khi cố gắng Takes một mutex nếu không có sẵn mutex.

Tính kế thừa mức ưu tiên: khi một Task có mức độ ưu tiên cao hơn bị Blocked khi cố gắng takes một mutex(token) hiện đang được giữ bởi một Task có mức độ ưu tiên thấp hơn, thì mức độ ưu tiên của Task đang giữ token sẽ được tạm thời nâng lên thành mức độ ưu tiên của Task bị Blocked. Cơ chế này được thiết kế để đảm bảo tác vụ có mức độ ưu tiên cao hơn được giữ ở trạng thái bị chặn trong thời gian ngắn nhất có thể, đồng thời giảm thiểu 'sự đảo ngược mức độ ưu tiên' đã xảy ra.

Mutexes không nên được sử dụng trong ISR vì:

- Bao gồm cơ chế kế thừa tính ưu tiên chỉ có ý nghĩa nếu mutex được gives và takes từ một Task chứ không phải ISR.
- ISR không thể bị Blocked để chờ tài nguyên được bảo vệ bởi mutex trở nên khả dụng.

![Alt text](https://www.freertos.org/fr-content-src/uploads/2018/07/mutexes.gif)

```C
Example usage:
SemaphoreHandle_t xSemaphore;

void vATask( void * pvParameters )
{
   /* Create a mutex type semaphore. */
   xSemaphore = xSemaphoreCreateMutex();

   if( xSemaphore != NULL )
   {
       /* The semaphore was created successfully and can be used. */
   }
}
```