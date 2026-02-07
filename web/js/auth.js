/**
 * 前端认证管理模块
 * 负责 Token 管理、路由保护、API 认证等
 */

class AuthManager {
    constructor() {
        this.tokenKey = 'authToken';
        this.usernameKey = 'username';
        this.userIdKey = 'userId';
    }
    
    /**
     * 获取存储的 Token
     */
    getToken() {
        return localStorage.getItem(this.tokenKey);
    }
    
    /**
     * 获取当前用户信息
     */
    getCurrentUser() {
        const token = this.getToken();
        if (!token) {
            return null;
        }
        
        return {
            userId: localStorage.getItem(this.userIdKey),
            username: localStorage.getItem(this.usernameKey),
            token: token
        };
    }
    
    /**
     * 检查用户是否已登录
     */
    isLoggedIn() {
        return !!this.getToken();
    }
    
    /**
     * 登出用户
     */
    logout() {
        localStorage.removeItem(this.tokenKey);
        localStorage.removeItem(this.usernameKey);
        localStorage.removeItem(this.userIdKey);
    }
    
    /**
     * 设置用户信息（登录时调用）
     */
    setUserInfo(token, username, userId) {
        localStorage.setItem(this.tokenKey, token);
        localStorage.setItem(this.usernameKey, username);
        localStorage.setItem(this.userIdKey, userId);
    }
    
    /**
     * 验证 Token 是否仍然有效
     */
    async verifyToken() {
        const token = this.getToken();
        if (!token) {
            return false;
        }
        
        try {
            const response = await fetch('/api/auth/verify', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ token })
            });
            
            return response.ok;
        } catch (error) {
            console.error('Token verification failed:', error);
            return false;
        }
    }
    
    /**
     * 检查受保护页面的访问权限
     * 如果未登录，自动重定向到登录页
     */
    async checkAuthRequired() {
        if (!this.isLoggedIn()) {
            window.location.href = 'login.html';
            return false;
        }
        
        // 验证 Token 是否有效
        const isValid = await this.verifyToken();
        if (!isValid) {
            this.logout();
            window.location.href = 'login.html';
            return false;
        }
        
        return true;
    }
    
    /**
     * 为 API 请求添加认证头
     * @param {object} options - fetch 选项
     */
    addAuthHeader(options = {}) {
        const token = this.getToken();
        if (!token) {
            return options;
        }
        
        if (!options.headers) {
            options.headers = {};
        }
        
        options.headers['Authorization'] = `Bearer ${token}`;
        return options;
    }
}

// 创建全局 authManager 实例
const authManager = new AuthManager();

/**
 * 增强的 fetch 函数，自动附加认证头
 */
async function authenticatedFetch(url, options = {}) {
    options = authManager.addAuthHeader(options);
    
    const response = await fetch(url, options);
    
    // 如果返回 401 Unauthorized，说明 Token 已失效，需要重新登录
    if (response.status === 401) {
        authManager.logout();
        window.location.href = 'login.html';
        throw new Error('Session expired, please login again');
    }
    
    return response;
}

/**
 * 显示登出按钮（在已登录页面上调用）
 */
function showLogoutButton() {
    const navbar = document.querySelector('.navbar');
    if (!navbar) return;
    
    const user = authManager.getCurrentUser();
    if (!user) return;
    
    // 查找导航栏右侧的菜单
    const navbarNav = navbar.querySelector('.navbar-collapse');
    if (!navbarNav) return;
    
    const navList = navbarNav.querySelector('.navbar-nav');
    if (!navList) return;
    
    // 添加用户信息显示和登出按钮
    const userItem = document.createElement('li');
    userItem.className = 'nav-item dropdown';
    userItem.innerHTML = `
        <a class="nav-link dropdown-toggle" href="#" id="userDropdown" role="button" data-bs-toggle="dropdown">
            👤 ${user.username}
        </a>
        <ul class="dropdown-menu dropdown-menu-end">
            <li><a class="dropdown-item" href="#" onclick="handleLogout()">退出登录</a></li>
        </ul>
    `;
    
    navList.appendChild(userItem);
}

/**
 * 处理登出
 */
function handleLogout() {
    if (confirm('确定要退出登录吗？')) {
        authManager.logout();
        window.location.href = 'login.html';
    }
}

/**
 * 页面加载时自动检查认证状态
 * 可在 DOMContentLoaded 中调用
 */
document.addEventListener('DOMContentLoaded', function() {
    // 如果页面需要认证但用户未登录，会自动重定向
    // 具体页面可以在其脚本中调用 checkAuthRequired()
});
