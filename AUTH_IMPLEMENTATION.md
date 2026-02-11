# 用户登录与注册功能实现完成

## 📋 实现概览

成功为简易博客系统实现了完整的**用户认证与授权系统**，包括用户注册、登录、会话管理和访问控制。

---

## ✨ 核心功能

### 1️⃣ 用户注册
- **接口**: `POST /api/auth/register`
- **请求**: `{"username": "账号", "password": "密码"}`
- **验证规则**:
  - ✅ 账号长度 ≥ 3 字符
  - ✅ 密码长度 ≥ 6 字符
  - ✅ 账号全局唯一（重复返回 409 Conflict）
- **返回**: JWT Token、用户 ID、用户名
- **密码处理**: SHA-256 哈希 + 随机 Salt

### 2️⃣ 用户登录
- **接口**: `POST /api/auth/login`
- **请求**: `{"username": "账号", "password": "密码"}`
- **验证流程**:
  1. 查询账号是否存在
  2. 对比密码哈希值
  3. 验证成功签发 JWT Token
- **返回**: JWT Token、用户 ID、用户名
- **错误处理**: 账号密码错误返回 401，不泄露具体信息

### 3️⃣ Token 验证
- **接口**: `POST /api/auth/verify`
- **用途**: 验证 JWT Token 是否有效且未过期
- **返回**: 用户 ID、用户名（如果有效）

### 4️⃣ 文章访问控制
- **受保护页面**: 
  - `index.html`（博客首页）- 需要登录才能访问
  - `editor.html`（文章发布页）- 需要登录才能访问

- **受保护 API**:
  - `POST /api/posts`（创建文章）
  - `PUT /api/posts/:id`（编辑文章）
  - `DELETE /api/posts/:id`（删除文章）

- **认证方式**: HTTP Header `Authorization: Bearer {token}`
- **权限规则**:
  - 普通用户可编辑/删除自己的文章
  - 管理员可查看/删除所有文章，但不能编辑他人文章
  - 管理员删除他人文章为软删除，记录 `deleted_by_admin` 与 `deleted_at`

---

## 📁 文件结构

### 后端新增文件

**加密与认证工具:**
- `server/include/crypto_utils.h` - SHA-256 哈希、Salt 管理
- `server/include/jwt_utils.h` - JWT 生成、验证、解析

**数据库扩展:**
- `server/include/database.h` - 添加 User 结构和软删除字段
- `server/src/database.cpp` - 实现用户 CRUD 操作与软删除逻辑

**API 端点:**
- `server/src/main.cpp` - 添加认证和文章保护接口

### 前端新增文件

**认证页面:**
- `web/login.html` - 登录表单页面
- `web/register.html` - 注册表单页面

**认证管理:**
- `web/js/auth.js` - Token 管理、路由保护、增强 fetch

**修改的页面:**
- `web/index.html` - 添加 auth.js 引入和登出按钮
- `web/editor.html` - 添加 auth.js 引入
- `web/js/index.js` - 添加认证检查和 authenticatedFetch
- `web/js/editor.js` - 添加认证检查和 authenticatedFetch

---

## 🛠️ 编译与运行

### 1. 编译项目
```bash
cd /Users/bytedance/Projects/Blog
./build.sh
```

### 2. 启动服务器
```bash
cd build
./bin/blog_server
```

### 3. 访问应用
- 首次访问自动重定向到登录页: http://localhost:8080
- 登录页面: http://localhost:8080/login.html
- 注册页面: http://localhost:8080/register.html

---

## 📊 API 端点汇总

| 方法 | 端点 | 认证 | 描述 |
|------|------|------|------|
| POST | `/api/auth/register` | ❌ | 用户注册 |
| POST | `/api/auth/login` | ❌ | 用户登录 |
| POST | `/api/auth/verify` | ❌ | Token 验证 |
| GET | `/api/posts` | ❌ | 获取所有文章 |
| GET | `/api/posts/:id` | ❌ | 获取单篇文章 |
| POST | `/api/posts` | ✅ | 创建文章 |
| PUT | `/api/posts/:id` | ✅ | 编辑文章 |
| DELETE | `/api/posts/:id` | ✅ | 删除文章 |

---

### 删除行为说明
- 管理员删除他人文章为软删除，作者仍可见该文章并可阅读/删除但不可编辑

## 🧪 测试案例

运行完整测试脚本：
```bash
chmod +x test-auth.sh
./test-auth.sh
```

**测试覆盖**:
- ✅ 用户注册成功
- ✅ 重复账号检测（409 冲突）
- ✅ 用户登录成功
- ✅ 错误密码检测（401 认证失败）
- ✅ Token 验证
- ✅ 无 Token 创建文章失败（401）
- ✅ 有效 Token 创建文章成功（201）

---

## 🎯 用户流程

### 首次使用
1. 用户访问 http://localhost:8080
2. 自动重定向到 login.html（因为未登录）
3. 用户点击"立即注册"进入 register.html
4. 填写账号和密码，点击注册
5. 后端验证账号唯一性，哈希密码并存储
6. 自动签发 JWT Token 并保存到 localStorage
7. 前端自动跳转到首页（index.html）

### 后续登录
1. 用户访问应用，自动检测 Token
2. 如果 Token 有效，直接进入首页
3. 如果 Token 过期或缺失，重定向到 login.html
4. 用户输入账号和密码登录
5. 后端验证并签发新 Token
6. 跳转到首页

### 发布文章
1. 用户在首页点击"发布文章"
2. 前端检查 Token，如果有效则进入编辑页
3. 用户填写标题和内容
4. 提交时，前端在 Authorization Header 中附加 Token
5. 后端验证 Token 并创建文章
6. 跳转回首页显示新文章

### 登出
1. 用户点击导航栏中的"退出登录"
2. 前端清除 localStorage 中的 Token
3. 重定向到登录页

---

## 🔧 配置与定制

### 修改 Token 过期时间
编辑 `server/include/jwt_utils.h`:
```cpp
const long long JwtUtils::EXPIRATION_TIME = 7 * 24 * 60 * 60;  // 修改此行
```

### 修改 JWT 签名密钥
编辑 `server/include/jwt_utils.h`:
```cpp
const std::string JwtUtils::SECRET_KEY = "your_secret_key_here";
```

### 修改密码规则
编辑 `server/src/main.cpp` 中的注册端点验证逻辑。

---

## 📦 依赖

### 后端依赖
- **OpenSSL** - 密码哈希和 JWT 签名
- **SQLite3** - 数据库
- **Crow** - Web 框架
- **nlohmann/json** - JSON 处理

### 前端依赖
- **Bootstrap 5.3** - UI 框架
- **现代浏览器** - ES6+ 支持

---

## ✅ 完成清单

- [x] 后端：密码哈希工具（SHA-256 + Salt）
- [x] 后端：JWT 工具（生成、验证、解析）
- [x] 数据库：创建 users 表
- [x] 后端：注册接口（账号唯一性检查）
- [x] 后端：登录接口（密码验证）
- [x] 后端：Token 验证接口
- [x] 后端：文章 API 添加 JWT 认证
- [x] 前端：登录页面
- [x] 前端：注册页面
- [x] 前端：Token 管理和路由保护（auth.js）
- [x] 前端：受保护页面检查
- [x] 前端：登出功能
- [x] 测试：功能完整性测试

---

## 🎉 总结

博客系统现已具备完整的认证和授权功能：

✨ **安全**：密码使用 SHA-256+Salt 加密，JWT Token 有 HMAC-SHA256 签名
🔐 **无状态**：使用 JWT 实现真正的无状态会话管理
🛡️ **受保护**：关键功能（创建/编辑/删除文章）受 Token 保护
👤 **用户友好**：清晰的注册/登录界面，自动会话管理
📱 **响应式**：前后端协作，无缝的用户体验

现在用户可以安全地创建账户、登录应用、发布和管理自己的文章了！
