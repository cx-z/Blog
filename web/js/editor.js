// 全局变量
let editingPostId = null;
let isEditMode = false;
let autoSaveTimer = null;
let lastSavedContent = '';

// 页面加载时检查认证
document.addEventListener('DOMContentLoaded', async function() {
    // 检查用户是否已登录
    const isAuthenticated = await authManager.checkAuthRequired();
    if (!isAuthenticated) {
        return;
    }
    
    // 显示用户信息和登出按钮
    showLogoutButton();
    
    // 检查是否为编辑模式
    const urlParams = new URLSearchParams(window.location.search);
    const mode = urlParams.get('mode');
    const postId = urlParams.get('id');
    
    if (mode === 'edit' && postId) {
        isEditMode = true;
        editingPostId = parseInt(postId);
        document.getElementById('page-title').textContent = '编辑文章';
        const canEdit = await loadPostForEditing(editingPostId);
        if (canEdit) {
            startAutoSave();
        }
    }
});

async function loadPostForEditing(postId) {
    try {
        const response = await authenticatedFetch(`/api/posts/${postId}`);
        const result = await response.json();
        
        if (result.success && result.data) {
            const post = result.data;
            const currentUser = authManager.getCurrentUser();
            const currentUserId = currentUser ? parseInt(currentUser.userId, 10) : null;
            const isAuthor = currentUserId !== null && post.user_id === currentUserId;
            if (!isAuthor) {
                isEditMode = false;
                editingPostId = null;
                showError('你没有权限编辑这篇文章');
                setTimeout(() => {
                    window.location.href = 'index.html';
                }, 1500);
                return false;
            }
            document.getElementById('post-title').value = post.title;
            document.getElementById('post-content').value = post.content;
            lastSavedContent = JSON.stringify({
                title: post.title,
                content: post.content
            });
            return true;
        } else {
            showError('无法加载文章内容，请稍后重试');
            return false;
        }
    } catch (error) {
        console.error('Error loading post:', error);
        showError('网络错误，请检查连接后重试');
        return false;
    }
}

function startAutoSave() {
    // 每30秒自动保存一次
    autoSaveTimer = setInterval(() => {
        autoSavePost();
    }, 30000);
}

async function autoSavePost() {
    if (!isEditMode || !editingPostId) {
        return;
    }
    
    const title = document.getElementById('post-title').value.trim();
    const content = document.getElementById('post-content').value.trim();
    
    if (!title || !content) {
        return;
    }
    
    // 检查内容是否有改动
    const currentContent = JSON.stringify({ title, content });
    if (currentContent === lastSavedContent) {
        return;
    }
    
    try {
        showAutoSaveStatus('自动保存中...');
        
        const response = await authenticatedFetch(`/api/posts/${editingPostId}`, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ title, content })
        });
        
        const result = await response.json();
        
        if (result.success) {
            lastSavedContent = currentContent;
            showAutoSaveStatus('已自动保存 ✓');
            
            // 3秒后隐藏提示
            setTimeout(() => {
                const statusEl = document.getElementById('auto-save-status');
                if (statusEl) {
                    statusEl.style.display = 'none';
                }
            }, 3000);
        } else {
            showAutoSaveStatus('自动保存失败，请检查网络');
        }
    } catch (error) {
        console.error('Error auto-saving post:', error);
        showAutoSaveStatus('自动保存失败');
    }
}

function showAutoSaveStatus(message) {
    let statusEl = document.getElementById('auto-save-status');
    if (!statusEl) {
        statusEl = document.createElement('div');
        statusEl.id = 'auto-save-status';
        statusEl.style.cssText = `
            position: fixed;
            top: 80px;
            right: 20px;
            background-color: #d4edda;
            color: #155724;
            padding: 12px 20px;
            border: 1px solid #c3e6cb;
            border-radius: 4px;
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
            z-index: 1000;
            font-size: 0.9rem;
        `;
        document.body.appendChild(statusEl);
    }
    
    statusEl.textContent = message;
    statusEl.style.display = 'block';
}

// 表单提交处理
document.getElementById('post-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    // 表单验证
    if (!document.getElementById('post-form').checkValidity()) {
        e.stopPropagation();
        document.getElementById('post-form').classList.add('was-validated');
        return;
    }
    
    const title = document.getElementById('post-title').value.trim();
    const content = document.getElementById('post-content').value.trim();
    
    if (!title || !content) {
        showError('标题和内容不能为空');
        return;
    }
    
    try {
        let response;
        
        if (isEditMode && editingPostId) {
            // 编辑模式：使用PUT请求
            response = await authenticatedFetch(`/api/posts/${editingPostId}`, {
                method: 'PUT',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ title, content })
            });
        } else {
            // 新建模式：使用POST请求
            response = await authenticatedFetch('/api/posts', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ title, content })
            });
        }
        
        const result = await response.json();
        
        if (result.success) {
            // 隐藏错误消息
            document.getElementById('error-message').style.display = 'none';
            
            if (isEditMode) {
                // 编辑模式：显示成功提示，停留在当前页面
                lastSavedContent = JSON.stringify({ title, content });
                showSuccessMessage('文章已保存！');
            } else {
                // 新建模式：显示成功消息并跳转
                document.getElementById('success-message').style.display = 'block';
                document.getElementById('post-form').reset();
                document.getElementById('post-form').classList.remove('was-validated');
                
                // 3 秒后跳转回首页
                setTimeout(() => {
                    window.location.href = 'index.html';
                }, 2000);
            }
        } else {
            showError(result.message || '操作失败，请重试');
        }
    } catch (error) {
        console.error('Error:', error);
        showError('网络错误或会话已过期，请重新登录');
    }
});

function showError(message) {
    document.getElementById('error-message').style.display = 'block';
    document.getElementById('error-text').textContent = message;
    document.getElementById('success-message').style.display = 'none';
}

function showSuccessMessage(message) {
    let successEl = document.getElementById('save-success-message');
    if (!successEl) {
        successEl = document.createElement('div');
        successEl.id = 'save-success-message';
        successEl.className = 'alert alert-success mt-3';
        successEl.style.display = 'block';
        document.getElementById('error-message').parentNode.insertBefore(
            successEl,
            document.getElementById('error-message').nextSibling
        );
    }
    
    successEl.textContent = message;
    successEl.style.display = 'block';
    
    // 5秒后隐藏
    setTimeout(() => {
        successEl.style.display = 'none';
    }, 5000);
}
