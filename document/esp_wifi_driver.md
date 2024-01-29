# Wi-Fi Driver

## ESP32 Wi-Fi Feature List

- 4 vitual Wi-Fi interface, STA(Station), AP(Access Point), Sniffer, Reserved.
- Station-only mode, AP-only mode, Station/AP-coexistence mode.
- IEEE 802.11b, IEEE 802.11g, IEEE 802.11n, và APIs để config protocol.
- WPA/WPA2/WPA3/WPA2-Enterprise/WPA3-Enterprise/WAPI/WPS and DPP.
- 20 MBit/s TCP throughput và 30 MBit/s UDP throughput over the air.
- Wi-Fi Aware (NAN).

## ESP32 Wi-Fi Programming Model

![Alt text](image-3.png)

`Wi-Fi Driver` được xem như là một `Black Box` không cần biết gì về các tầng code cao hơn (TCP stack, Event task, Applocation task). `Application task` sử dụng các APIs để khởi tạo Wi-Fi và xử lý các Wi-Fi Events khi cần thiết. `Wi-Fi Driver` tiếp nhận APIs call, xử lý chúng và post Event lên Appliation.

Wi-Fi Event handling nằm trong thư viện `esp_event`. Các Event được Wi-Fi driver gửi tới default event loop. Application có thể xử lý các sự kiện này trong các hàm callback được đăng ký bằng cách sử dụng hàm `esp_event_handler_register()`. Wi-Fi event cũng được xử lý bởi esp_netif để cung cấp các hành vi mặc định. Ví dụ như một Wi-Fi Station kết nối với một AP, esp_netif sẽ tự động khởi động DHCP client theo mặc định.

## ESP32 Wi-Fi Station General Scenario

![Alt text](image-4.png)

#### 1. Wi-Fi/LwIP Init Phase

**Step 1:** Main task gọi hàm `esp_netif_init()` để tạo LwIP core task và khởi tạo LwIP-related work.

**Step 2:** Main task gọi hàm `esp_event_loop_create()` để tạo hệ thống Event Task và khởi tạo các hàm call back cho applocation. Khi sử cấu hình là Wi-Fi station thì các application event callback không làm gì ngoài việc chuyển tiếp các events đến các application task.

**Step 3:** Main task gọi hàm `esp_netif_create_default_wifi_ap()` hoặc `esp_netif_create_default_wifi_sta()` để tạo một station hoặc access point liên kết default network interface với TCP/IP stack.

**Step 4:** Main task gọi hàm `esp_wifi_init()` để tạo Wi-Fi Driver task và Wi-Fi Driver.

```C
void app_main(void)
{
    /*--Wi-Fi/LwIP Init Phase--*/
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_init_conf = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init_conf);
}
```

#### 2. Wi-Fi Configuration Phase

Sau khi đã khởi tạo Wi-Fi Driver, ta có thể bắt đầu configuring Wi-Fi Driver.

- Cấu hình Wi-Fi là Station, gọi hàm `esp_wifi_set_mode()` `(WIFI_MODE_STA)`.
- Nói chung, Wi-Fi Driver phải được config trước khi thiết lập kết nối Wi-Fi. Nhưng điều này KHÔNG bắt buộc, ta có thể config Wi-Fi connection bất kỳ lúc nào, miễn là Wi-Fi Driver được khởi tạo thành công. Tuy nhiên, nếu cấu hình không cần thay đổi sau khi kết nối Wi-Fi được thiết lập, nên config Wi-Fi Driver ở giai đoạn này, vì các API cấu hình (chẳng hạn như `esp_wifi_set_protocol()`) sẽ khiến Wi-Fi kết nối lại.
- Nếu Wi-Fi NVS flash được enabled, tất cả Wi-Fi configuration ở phase này, hoặc phase sau, sẽ được lưu vào trong flash. Khi board reset, ta không cần config Wi-Fi Driver lại từ đầu. Chỉ cần gọi các API `esp_wifi_get_xxx` để tìm nạp cấu hình được lưu trữ trong flash trước đó.

```C
void app_main(void)
{
    /*---Wi-Fi Configuration Phase---*/
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_conf = {
    // Config Station
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            // Configuration for Protected Management Frame. Will be advertised in RSN Capabilities in RSN IE.
            .pmf_cfg = {
            .capable = true,
            .required = false
            }
        }
    };

    esp_wifi_set_config(WIFI_IF_STA, &wifi_conf);
}
```

#### 3. Wi-Fi Start Phase

**Step 1:** Gọi hàm `esp_wifi_start()`, để khởi động Wi-Fi Driver.
**Step 2:** Wi-Fi Driver post `WIFI_EVENT_STA_START` đến event task, nó sẽ thực hiện một số việc phổ biến và sẽ gọi các hàm event callback.
**Step 3:** Hàm application event callback chuyển tiếp `WIFI_EVENT_STA_START` đến application task.

```C
void app_main(void)
{
    /*---Wi-fi eventgroup---*/
    // Register wi-fi event
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);

    /*---Wi-Fi Start Phase---*/
    esp_wifi_start()
}

static void wifi_event_handler( void *arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START))
    {
        esp_wifi_connect();
    }
}
```

#### 4. Wi-Fi Connect Phase

**Step 1:** Khi hàm `esp_wifi_connect()` được gọi, Wi-Fi Driver bắt đầu quá trình scan/connection.
**Step 2:** Nếu quá trình scan/connection thành công, `WIFI_EVENT_STA_CONNECTED` sẽ được tạo ra, trong Event Task sẽ khởi động DHCP client, cuối cùng kích hoạt quy trình DHCP. Nếu connect fail, `WIFI_EVENT_STA_DISCONNECTED` sẽ được tạo ra và lý do lỗi cũng sẽ được cung cấp.

```C
#define WIFI_MAXIMUM_RETRY_CONNECT 3
static uint8_t retry_connect_count = 0;

static void wifi_event_handler( void *arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START))
    {
        esp_wifi_connect();
    }
    /*---Wi-Fi Connect Phase---*/
    else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        if(retry_connect_count < WIFI_MAXIMUM_RETRY_CONNECT)
        {
            esp_wifi_connect();
            retry_connect_count++;
            ESP_LOGI("Retry to connect to the Access Point");
        }else
        {
        ESP_LOGI("Fail to connect the Access Point");
        retry_connect_count = 0;
        }
    }
}
```

#### 5. Wi-Fi ‘Got IP’ Phase

**Step 1:** Khi DHCP client đã được khởi tạo, `Got IP` phase sẽ bắt đầu.

**Step 2:** Nếu IP address được nhận thành công từ DHCP server, `IP_EVENT_STA_GOT_IP` sẽ phát sinh và event task sẽ handle.

**Step 3:** Trong application event callback, `IP_EVENT_STA_GOT_IP` sẽ được chuyển tiếp đến các application task. Đối với các ứng dụng dựa trên LwIP, sự kiện này rất đặc biệt và có nghĩa là mọi thứ đã sẵn sàng để ứng dụng bắt đầu các tác vụ của nó, ví dụ: tạo ổ cắm TCP/UDP.

```C
void app_main(void)
{
    /*---Wi-fi eventgroup---*/
    // Register wi-fi event
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);

    // Register ip event
    esp_event_handler_instance_t instance_any_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_ip);

    /*---Wi-Fi Start Phase---*/
    esp_wifi_start()
}

static void wifi_event_handler( void *arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START))
    {
        esp_wifi_connect();
    }
    /*---Wi-Fi Connect Phase---*/
    else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        if(retry_connect_count < WIFI_MAXIMUM_RETRY_CONNECT)
        {
            esp_wifi_connect();
            retry_connect_count++;
            ESP_LOGI("Retry to connect to the Access Point");
        }else
        {
        ESP_LOGI("Fail to connect the Access Point");
        retry_connect_count = 0;
        }
    }
    /*---Wi-Fi ‘Got IP’ Phase---*/
    else if((event_base == WIFI_EVENT) && (event_id == IP_EVENT_STA_GOT_IP))
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_connect_count = 0;
    }
}
```
## 6. Wi-Fi Disconnect Phase

**Step 1:** Khi kết nối Wi-Fi bị gián đoạn, ví dụ: AP bị tắt nguồn hoặc RSSI kém, `WIFI_EVENT_STA_DISCONNECTED` sẽ phát sinh. Ở đây, event task sẽ thông báo cho LwIP Task để clear/remove tất cả UDP/TCP connection. Sau đó, tất cả các application socket sẽ ở wrong status. Nói cách khác, không có socket nào có thể hoạt động bình thường khi event này xảy ra.

**Step 2:** Application event callback function sẽ chuyển tiếp `WIFI_EVENT_STA_DISCONNECTED` đến application task. Có thể handle như sau:
- Gọi lại `esp_wifi_connect()` để kết nối lại.
- Đóng tất cả các socket.
- Khởi tạo lại nếu cần.

## 7. Wi-Fi IP Change Phase

**Step 1:** Nếu IP Address bị thay đổi, `IP_EVENT_STA_GOT_IP` sẽ phát sinh và `ip_change` được set thành `true`.

**Step 2:** Event này rất quan trọng đối với application. Khi nó xảy ra, ta phải đóng tất cả socket và khởi tạo lại chúng.

## 7. Wi-Fi Deinit Phase

```C
void app_main(void)
{
    // Disconnect the Wi-Fi connectivity
    esp_wifi_disconnect();
    // stop the Wi-Fi Driver
    esp_wifi_stop();
    // Unload the Wi-Fi Driver
    esp_wifi_deinit();
}
```