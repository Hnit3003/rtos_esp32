# Queue

## FreeRTOS Queue - Inter-task communication and synchronisation

> Queue là hình thức giao tiếp chính giữa các Task với nhau. Có thể dùng Queue để trao đổi dữ liệu giữa các Task, hoặc giữa ngắt (ISR) với Task.

![Alt text](https://www.freertos.org/fr-content-src/uploads/2018/07/queue_animation.gif)

- Queue API cho phép chỉ định các Task vào Blocked state.
- Khi một Task cố đọc một Empty Queue thì Task đó sẽ được đưa vào Blocked state cho đến khi có dữ liệu được truyền vào Queue.
- Khi một Task cố đọc một Full Queue thì Task đó cũng sẽ được đưa vào Blocked state cho đến khi Queue có chỗ cho dữ liệu mới được vào.
- Nếu có nhiều Task cùng bị Blocked do một Queue thì Task có mức độ ưu tiên cao hơn sẽ được Unblocked trước.
- Các hàm của Queue được sử dụng trong ISR phải là các hàm có kết thúc bằng `FromISR`.

## Function of FreeRTOS Queue

#### **`xQueueCreate`**

> Tạo một Queue và trả về Handle mà Queue có thể được tham chiếu tới.
Mỗi Queue cần RAM để giữ trạng thái Queue, và giữ dữ liệu được lưu trữ trong nó. Hàm `xQueueCreate()` sẽ tự động cấp phát bộ vùng nhớ Heap cho Queue.

```C
QueueHandle_t xQueueCreate( UBaseType_t uxQueueLength,
                            UBaseType_t uxItemSize );
```

*Parameter:*

- **`uxQueueLength`**: Số phần tử tối đa có mà Queue có thể chứa.
- **`uxItemSize`**: Kích thước của từng phần tử trong Queue, tính theo Byte. Phần tử được sẽ được copy rồi truyền vào Queue, không phải truyền tham chiếu, vì vậy đây là số byte sẽ được copy đối với mỗi phần tử. Các phần tử trong Queue phải có cùng kích thước.

*Return:*

- Nếu tạo Queue thành công sẽ trả về một Handle của nó. Nếu không thể cấp phát bộ nhớ để tạo Queue, trả về NULL.

*Example:*

```C
QueueHandle_t xQueue1;
xQueue1 = xQueueCreate(10, sizeof(unsigned long));
if(xQueue1 == NULL)
{
    //Queue was not created.
}
```


#### **`vQueueDelete`**

> Xóa Queue - free vùng Heap đã cấp phát cho Queue.

```C
void vQueueDelete( QueueHandle_t xQueue );
```

*Parameter:*

- **`xQueue`**: Handle của Queue.

#### **`vQueueReset`**

> Reset Queue về trạng thái Empty như ban đầu.

```C
BaseType_t xQueueReset( QueueHandle_t xQueue );
```

*Parameter:*

- **`xQueue`**: Handle của Queue.

*Return:*

- Luôn return pdPASS.

#### **`uxQueueMessagesWaiting`**

> Trả về số phần tử mà Queue đang chứa.

```C
UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
```

*Parameter:*

- **`xQueue`**: Handle của Queue.

*Return:*

- Số phần tử mà Queue đang chứa.

#### **`uxQueueMessagesWaiting--FromISR`**

> Trả về số phần tử mà Queue đang chứa. Nếu có `FromISR` ở cuối thì hàm này nên được sử dụng trong ISR.

```C
UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
```

```C
UBaseType_t uxQueueMessagesWaitingFromISR( QueueHandle_t xQueue );
```

*Parameter:*

- **`xQueue`**: Handle của Queue.

*Return:*

- Số phần tử mà Queue đang chứa.

#### **`uxQueueSpacesAvailable`**

> Trả về dung lượng còn trống của Queue.

```C
UBaseType_t uxQueueSpacesAvailable( QueueHandle_t xQueue );
```

*Parameter:*

- **`xQueue`**: Handle của Queue.

*Return:*

- Dung lượng còn trống của Queue.

#### **`xQueueIsQueueEmptyFromISR / xQueueIsQueueFullFromISR`**

> Truy vấn Queue để xem Queue có Empty không. Chỉ sử dụng hàm này cho ISR.

```C
BaseType_t xQueueIsQueueEmptyFromISR( const QueueHandle_t xQueue );
```

```C
BaseType_t xQueueIsQueueFullFromISR( const QueueHandle_t xQueue );
```

*Parameter:*

- **`xQueue`**: Handle của Queue.

*Return:*

- `pdFALSE`
- `pdTRUE`


#### **`xQueueSend`**

> Đây là một macro gọi `xQueueGenericSend()`.
> Dùng để post một phần tử dữ liệu vào cuối Queue, phần tử được truyền theo kiểu tham trị, không phải tham chiếu. Khi ngắt (ISR), ta phải gọi hàm `xQueueSendFromISR`.

```C
BaseType_t xQueueSend(  QueueHandle_t xQueue,
                        const void * pvItemToQueue,
                        TickType_t xTicksToWait );
```

*Parameter:*

- **`xQueue`**: Handle của Queue muốn post dữ liệu.
- **`pvItemToQueue`**: Con trỏ trỏ tới dữ liệu được post.
- **`xTicksToWait`**: Thời gian tối đa mà Task có thể vào Blocked state nếu Queue đang full. Việc gọi hàm sẽ ngay lập tức return và reset `xTickToWait` về `0`. Cần dùng hằng số `portTICK_PERIOD_MS` để chuyển thời gian về milisecond. Có thể chỉ định thời gian Blocked là `portMAX_DELAY` để Task có thể ở Blocked vô thời hạn (không có thời gian chờ).

*Return:*

- `errQUEUE_FULL`: Queue full, không thể post thêm dữ liệu. 
- `pdTRUE`: Nếu phần tử post vào Queue thành công.

*Example:*

```C
QueueHandle_t xQueue1;
xQueue1 = xQueueCreate( 10, sizeof( unsigned long ) );
unsigned long ulVar = 10UL

if(xQueue1 != NULL)
{
    if((xQueueSend(xQueue1, (void *)&ulVar), (TickType_t)10) != pdTRUE)
    {
        //Failed to post the message, even after 10 ticks.
    }
}
```

#### **`xQueueSendFromISR`**

> Đây là một macro gọi `xQueueGenericSendFromISR()`.
> Dùng để post một phần tử dữ liệu vào cuối Queue, hàm này an toàn khi sử dụng trong ngắt(ISR).
> Dữ liệu được truyền vào Queue bằng cách tham chiếu, không phải tham trị. Nên tốt hơn truyền các phần tử với kích thước nhỏ, do quá trình diễn ra trong ngắt phải nhanh. Nên lưu trữ con trỏ trỏ đến địa chỉ của dữ liệu cần truyền.

```C
 BaseType_t xQueueSendFromISR(  QueueHandle_t xQueue,
                                const void *pvItemToQueue,
                                BaseType_t *pxHigherPriorityTaskWoken   );
```

*Parameter:*

- **`xQueue`**: Handle của Queue muốn post dữ liệu.
- **`pvItemToQueue`**: Con trỏ trỏ tới dữ liệu được post.
- **`pxHigherPriorityTaskWoken`**: `xQueueSendFromISR()` sẽ set `*pxHigherPriorityTaskWoken` thành `pdTRUE` nếu việc gửi tới Queue làm cho một Task switched out khỏi Blocked state và Task này có mức độ ưu tiên cao hơn Task hiện đang Running. Nếu `xQueueSendFromISR()` đặt giá trị này thành pdTRUE thì nên yêu cầu chuyển ngữ cảnh trước khi thoát khỏi ngắt. Ta có thể để thông số là `NULL`.

*Return:*

- `errQUEUE_FULL`: Queue full, không thể post thêm dữ liệu. 
- `pdTRUE`: Nếu phần tử post vào Queue thành công.

#### **`xQueueSendToBack--FromISR`**

> Đây là một macro gọi `xQueueGenericSend--FromISR()`, tương đương với việc gọi hàm `xQueueSend()`.
> Dùng để post một phần tử dữ liệu vào cuối của Queue, phần tử được truyền theo kiểu tham trị, không phải tham chiếu. Khi ngắt (ISR), ta phải gọi hàm `xQueueSendToBackFromISR`.

```C
BaseType_t xQueueSendToBack(    QueueHandle_t xQueue,
                                const void * pvItemToQueue,
                                TickType_t xTicksToWait );
```

```C
BaseType_t xQueueSendToBackFromISR(     QueueHandle_t xQueue,
                                        const void * pvItemToQueue,
                                        TickType_t xTicksToWait );
```

*Parameter:*

- **`xQueue`**: Handle của Queue chứa phần tử được post.
- **`pvItemToQueue`**: Con trỏ trỏ tới phần tử được post.
- **`xTicksToWait`**: Thời gian tối đa mà Task có thể vào Blocked state nếu Queue đang full. Việc gọi hàm sẽ ngay lập tức return và reset `xTickToWait` về `0`. Cần dùng hằng số `portTICK_PERIOD_MS` để chuyển thời gian về milisecond. Có thể chỉ định thời gian Blocked là `portMAX_DELAY` để Task có thể ở Blocked vô thời hạn (không có thời gian chờ).

*Return:*

- `errQUEUE_FULL`: Queue full, không thể post thêm dữ liệu. 
- `pdTRUE`: Nếu phần tử post vào Queue thành công.

#### **`xQueueSendToFront--FromISR`**

> Đây là một macro gọi `xQueueGenericSend-FromISR()`.
> Dùng để post một phần tử dữ liệu vào đầu của Queue, phần tử được truyền theo kiểu tham trị, không phải tham chiếu. Khi ngắt (ISR), ta phải gọi hàm `xQueueSendToFrontFromISR` 

```C
BaseType_t xQueueSendToFront(   QueueHandle_t xQueue,
                                const void * pvItemToQueue,
                                TickType_t xTicksToWait );
```

```C
BaseType_t xQueueSendToFrontFromISR(    QueueHandle_t xQueue,
                                        const void * pvItemToQueue,
                                        TickType_t xTicksToWait );
```

*Parameter:*

- **`xQueue`**: Handle của Queue chứa phần tử được post.
- **`pvItemToQueue`**: Con trỏ trỏ tới phần tử được post.
- **`xTicksToWait`**: Thời gian tối đa mà Task có thể vào Blocked state nếu Queue đang full. Việc gọi hàm sẽ ngay lập tức return và reset `xTickToWait` về `0`. Cần dùng hằng số `portTICK_PERIOD_MS` để chuyển thời gian về milisecond. Có thể chỉ định thời gian Blocked là `portMAX_DELAY` để Task có thể ở Blocked vô thời hạn (không có thời gian chờ).

*Return:*

- `errQUEUE_FULL`: Queue full, không thể post thêm dữ liệu. 
- `pdTRUE`: Nếu phần tử post vào Queue thành công.


#### **`xQueueReceive`**

> Đây là một macro gọi `xQueueGenericReceive()`.
> Dùng để đọc một phần tử dữ liệu từ Queue, phần tử nhận được là một bản sao nên phải biến đệm phải có kích thước đủ. Số byte nhận được sẽ bằng với dung lượng mỗi phần tử của Queue khi tạo Queue. Khi ngắt (ISR), ta phải gọi hàm `xQueueReceiveFromISR`. 

```C
BaseType_t xQueueReceive(   QueueHandle_t xQueue,
                            void *pvBuffer,
                            TickType_t xTicksToWait );
```

*Parameter:*

- **`xQueue`**: Handle của Queue muốn đọc dữ liệu.
- **`pvBuffer`**: Con trỏ trỏ tới biến đệm mà phần tử được copy vào.
- **`xTicksToWait`**: Thời gian tối đa mà Task có thể vào Blocked state nếu Queue đang trống. Đặt `xTickToWait` thành `0` sẽ khiến hàm return ngay lập tức nếu Queue trống. Cần dùng hằng số `portTICK_PERIOD_MS` để chuyển thời gian về milisecond. Có thể chỉ định thời gian Blocked là `portMAX_DELAY` để Task có thể ở Blocked vô thời hạn (không có thời gian chờ).

*Return:*

- `pdFALSE`: Không đọc được dữ liệu từ Queue. 
- `pdTRUE`: Nếu phần tử đọc được từ Queue thành công.

#### **`xQueueReceiveFromISR`**

> Dùng để đọc một phần tử dữ liệu từ Queue, hàm này an toàn khi gọi trong ngắt(ISR).

```C
 BaseType_t xQueueReceiveFromISR(   QueueHandle_t xQueue,
                                    void *pvBuffer,
                                    BaseType_t *pxHigherPriorityTaskWoken   );
```

*Parameter:*

- **`xQueue`**: Handle của Queue muốn đọc dữ liệu.
- **`pvBuffer`**: Con trỏ trỏ tới biến đệm mà phần tử được copy vào.
- **`pxHigherPriorityTaskWoken`**: Một Task có thể switched in Blocked state vì đang chờ một Queue trống. Nếu `xQueueReceiveFromISR()` làm cho Task đó switched out Blocked state thì `*pxHigherPriorityTaskWoken` sẽ được set thành pdTRUE, nếu không thì giá trị con trỏ này trỏ đến không thay đổi.

*Return:*

- `pdFALSE`: Không đọc được dữ liệu từ Queue. 
- `pdTRUE`: Nếu phần tử đọc được từ Queue thành công.
- 
#### **`xQueueOverwrite`**

> Đây là một macro để gọi hàm `xQueueGenericSend()`.
> Là một version của `xQueueSendToBack()`, nó sẽ ghi một dữ liệu vào queue bất kể queue đã full, ghi đè lên dữ liệu đang được lưu trong Queue.
> Ta thường dùng hàm này cho các Queue có số phần tử tối đa là 1, các Queue này chỉ có hai trạng thái: full hoặc empty. Khi ngắt(ISR), ta phải sử dụng hàm `xQueueOverwriteFromISR()`.

```C
 BaseType_t xQueueOverwrite(    QueueHandle_t xQueue,
                                const void * pvItemToQueue  );
```

*Parameter:*

- **`xQueue`**: Handle của Queue muốn đọc dữ liệu.
- **`pvItemToQueue`**: Con trỏ trỏ tới biến dữ liệu sẽ được overwrite vào Back của Queue. Số byte được copy từ biến vào Queue sẽ được xác định khi tạo Queue `xQueueCreate()`.

*Return:*

- `pdPASS`: Luôn được trả về do hàm này làm cho dữ liệu luôn được truyền vào Queue.

#### **`xQueueOverwriteFromISR`**

> Đây là một macro để gọi hàm `xQueueGenericSendFromISR()`.
> Là một version của `xQueueSendToBackFromISR()`, nó sẽ ghi một dữ liệu vào queue bất kể queue đã full, ghi đè lên dữ liệu đang được lưu trong Queue. Hàm này an toàn khi sử dụng trong ngắt (ISR).
> Ta thường dùng hàm này cho các Queue có số phần tử tối đa là 1, các Queue này chỉ có hai trạng thái: full hoặc empty. Khi ngắt(ISR), ta phải sử dụng hàm `xQueueOverwriteFromISR()`.

```C
BaseType_t xQueueOverwrite(     QueueHandle_t xQueue,
                                const void * pvItemToQueue
                                BaseType_t *pxHigherPriorityTaskWoken   );
```

*Parameter:*

- **`xQueue`**: Handle của Queue muốn đọc dữ liệu.
- **`pvItemToQueue`**: Con trỏ trỏ tới biến dữ liệu sẽ được overwrite vào Back của Queue. Số byte được copy từ biến vào Queue sẽ được xác định khi tạo Queue `xQueueCreate()`.
- **`pxHigherPriorityTaskWoken`**: Hàm này sẽ set `pxHigherPriorityTaskWoken` là `pdTRUE` nếu việc gửi dữ liệu tới Queue làm cho một Task được unblocked, và Task này có mức ưu tiên cao hơn Task đang ở Running state. 

*Return:*

- `pdPASS`: Luôn được trả về do hàm này làm cho dữ liệu luôn được truyền vào Queue.

#### **`xQueuePeek`**

> Đây là một macro để gọi hàm `xQueueGenericReceive()`.
> Đọc một dữ liệu từ Queue mà không xóa dữ liệu đó khỏi Queue. Dữ liệu nhận được bằng cách copy từ Queue, nên biến đệm lưu phải có kích thước phù hợp. Số byte được copy sẽ được xác định lúc tạo Queue.
> Nếu nhận thành công thì dữ liệu vẫn được giữ và sẽ được đọc lại nếu gọi lại hàm vào lần tiếp, hoặc nhận được bởi hàm `xQueueReceive().`

```C
BaseType_t xQueuePeek(  QueueHandle_t xQueue,
                        void *pvBuffer,
                        TickType_t xTicksToWait );
```

*Parameter:*

- **`xQueue`**: Handle của Queue muốn đọc dữ liệu.
- **`pvBuffer`**: Con trỏ trỏ tới biến đệm được dùng để lưu dữ liệu nhận được từ Queue.
- **`xTicksToWait`**: Thời gian tối đa mà Task sẽ bị Blocked để chờ dữ liệu được đưa vào Queue nếu Queue đang empty. Ta cần sử dụng hằng số portTICK_PERIOD_MS để chuyển đổi sang milisecond. Blocked time có thể set bằng portMAX_DELAY    sẽ cho phép Task blocked mà không cần timeout.

*Return:*

- `pdTRUE`: Nếu xem được dữ liệu từ Queue.
- `pdFALSE`: Nếu không xem được dữ liệu từ Queue.

#### **`xQueuePeekFromISR`**

> Hàm này được sử dụng trong ngắt(ISR).
> Đọc một dữ liệu từ Queue mà không xóa dữ liệu đó khỏi Queue. Dữ liệu nhận được bằng cách copy từ Queue, nên biến đệm lưu phải có kích thước phù hợp. Số byte được copy sẽ được xác định lúc tạo Queue.
> Nếu nhận thành công thì dữ liệu vẫn được giữ và sẽ được đọc lại nếu gọi lại hàm vào lần tiếp, hoặc nhận được bởi hàm `xQueueReceive--FromISR().`

```C
 BaseType_t xQueuePeekFromISR(  QueueHandle_t xQueue,
                                void *pvBuffer   );
```

*Parameter:*

- **`xQueue`**: Handle của Queue muốn đọc dữ liệu.
- **`pvBuffer`**: Con trỏ trỏ tới biến đệm được dùng để lưu dữ liệu nhận được từ Queue.

*Return:*

- `pdTRUE`: Nếu xem được dữ liệu từ Queue.
- `pdFALSE`: Nếu không xem được dữ liệu từ Queue. 