# [Telegraph](runImage/README.md)

##### 1. Loginthread模块：负责登录业务。

业务流程：`startConnect` 作为登录触发插槽，启动socket。绑定socket的 `connected`和 `disconnected` 信号，对应处理函数 `socket_Connected`和 `socket_Disconnected`

* `startConnect` ：登录业务处理函数，连接服务器，监听连接信号。
* `socket_Connected` ：1）开启监听信息接收，绑定 `socket_Read_Data ` 信号；2）发送登录账号密码。
* `socket_Read_Data` ：登录信息接收处理，对应触发主窗口信号。

##### 2. MainWindow模块：登录主业务

业务流程：

1. 登陆业务：创建线程，初始化登录模块，绑定线程。线程开始信号 `startThread` 绑定 `startConnect` ，绑定登录业务结果 `loginFailed` 和 `loginSuccess`。
2. 注册业务：类似登录业务，在主线程进行。

##### 3. RegWindow模块：负责注册业务

总体类似登录，发送消息采用json，字段为注册信息和一个 `action`字段指定动作类型。

---

##### 4. Home模块：交互业务

业务流程：

1. 用户上线：根据登录生成的 `token` 授权。线程执行，`messageThread` 事件进行处理验证。

##### 5. MessageThread模块：（用户信息业务）？

* `MessageThread` ：`socket`连接并设置连接与断开处理。
* `socket_Connected`：设置开启监听 `socket`接收文本与二进制数据的处理，发送 `action：auth` 消息，用户上线。
* `Receive` ：根据接收到消息的 `action：`执行不同的操作，`updateList`更新列表。。。。。。。。。。。。。。。。。。。。。。。。。。。

##### 6. UserItem模块：用户列表业务

* `unread` ：记录未读用户消息。
* `readAll` ：已读所有消息。

---

---

---

# [Server](runImage/README.md)

##### 1. MainWindos模块：主业务

业务流程：开启socket监听，注册 `newConnection` 信号与 槽 `server_New_Connect` ，登录数据库。根据接收的不同信息设置对应的槽处理。服务器采用一个 `connectionList` 来管理所有的socket连接，`onlineList` 记录所有在线用户。

* `onlineList` ：采用 `QHash<QString, User>`，key为用户名，value为用户信息。
* `Log` ：日志显示，采用时间+日志的格式。如：`[2020:10:10] this is log.`
* `server_New_Connect`：新连接处理函数，根据二进制数据还是文本数据设置不同处理槽。`textMessageReceived`-->`socket_Read_Data` ，`binaryMessageReceived`-->`socket_Read_File`，`disconnected`-->`socket_Disconnected`
* `socket_Disconnected`：用户下线，从 `onlineList` 中删除，广播下线消息。
* `socket_Read_Data`
  * **消息处理：** 该函数检查接收到的消息是否非空，如果消息长度在某个限制内（5000 字符以内），则使用 `Log` 函数记录消息，并使用 `handleReq` 函数进行处理。
  * **响应准备：** 该函数准备一个 JSON 格式的响应（`newMessage`），其中包括一个错误标志和消息。如果存在错误消息（`errorMsg` 非空），则将其包含在响应中。
  * **发送回复：** 如果 `wiiRepaly` 不等于 0，表示应该发送回复，该函数将响应转换为 JSON 字符串，并使用 WebSocket 客户端的 `sendTextMessage` 方法将其发送回客户端（`pClient`）。
  * **状态更新：** 函数末尾将 `wiiRepaly` 设置为 1，表示已发送回复。
* `handleReq`
  * **JSON解析：** 使用 `QJsonDocument::fromJson`将收到的字符串数据解析为JSON文档。
  * **处理操作：** 根据JSON文档中的 "action" 值执行相应的操作，比如注册新用户 (`registerNewUser`)、登录 (`Login`)、认证 (`addNewUser`)、发送文本消息 (`sendMsg`) 等。。。。。。。。。。。
  * **错误处理：** 如果解析失败或存在错误，设置 `errorMsg` 以指示无法解析请求。

##### 2. user模块：
