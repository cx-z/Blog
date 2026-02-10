// 页面加载时检查认证并获取文章列表
document.addEventListener('DOMContentLoaded', async function() {
    // 检查用户是否已登录
    const isAuthenticated = await authManager.checkAuthRequired();
    if (!isAuthenticated) {
        return;
    }
    
    // 显示用户信息和登出按钮
    showLogoutButton();
    
    // 加载文章列表
    loadPosts();
});

async function loadPosts() {
    const container = document.getElementById('posts-container');
    
    try {
        const response = await authenticatedFetch('/api/posts');
        const result = await response.json();
        const isAdmin = authManager.getRole() === 'admin';
        
        if (result.success && result.data.length > 0) {
            container.innerHTML = result.data.map(post => `
                <div class="card mb-4 post-card">
                    <div class="card-body position-relative">
                        <button class="btn-delete-post" onclick="deletePost(${post.id}); return false;" title="删除文章">×</button>
                        ${post.is_author ? `<button class="btn-edit-post" onclick="editPost(${post.id}); return false;" title="编辑文章">✎</button>` : ''}
                        ${isAdmin ? `<div class="post-author">${escapeHtml(getAuthorDisplayName(post))}</div>` : ''}
                        <h5 class="card-title">
                            <a href="#" onclick="showPost(${post.id}); return false;">
                                ${escapeHtml(post.title)}
                            </a>
                        </h5>
                        <p class="card-text text-muted">
                            <small>${formatDate(post.timestamp)}</small>
                        </p>
                        <p class="card-text">
                            ${escapeHtml(post.content.substring(0, 150))}...
                        </p>
                        <a href="#" onclick="showPost(${post.id}); return false;" class="btn btn-sm btn-outline-primary">
                            阅读更多
                        </a>
                    </div>
                </div>
            `).join('');
        } else {
            container.innerHTML = '<p class="text-center text-muted mt-5">还没有文章，<a href="editor.html">去发布第一篇</a></p>';
        }
    } catch (error) {
        console.error('Error loading posts:', error);
        container.innerHTML = '<div class="alert alert-danger">加载文章失败，请稍后重试</div>';
    }
}

function showPost(id) {
    // 这里可以使用模态框显示完整内容，或者跳转到详情页
    // 简单起见，使用 alert 展示
    authenticatedFetch(`/api/posts/${id}`)
        .then(response => response.json())
        .then(result => {
            if (result.success) {
                const post = result.data;
                const modal = new bootstrap.Modal(document.createElement('div'));
                const isAdmin = authManager.getRole() === 'admin';
                
                // 创建模态框
                const div = document.createElement('div');
                div.innerHTML = `
                    <div class="modal fade" id="postModal" tabindex="-1">
                        <div class="modal-dialog modal-lg">
                            <div class="modal-content">
                                <div class="modal-header">
                                    <h5 class="modal-title">${escapeHtml(post.title)}</h5>
                                    <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
                                </div>
                                <div class="modal-body">
                                    ${isAdmin ? `<p class="text-muted mb-1"><small>${escapeHtml(getAuthorDisplayName(post))}</small></p>` : ''}
                                    <p class="text-muted"><small>${formatDate(post.timestamp)}</small></p>
                                    <div class="post-content" style="white-space: pre-wrap;">
                                        ${escapeHtml(post.content)}
                                    </div>
                                </div>
                                <div class="modal-footer">
                                    <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">关闭</button>
                                </div>
                            </div>
                        </div>
                    </div>
                `;
                document.body.appendChild(div);
                
                const postModal = new bootstrap.Modal(div.querySelector('#postModal'));
                postModal.show();
                
                // 模态框关闭后移除元素
                div.querySelector('#postModal').addEventListener('hidden.bs.modal', () => {
                    div.remove();
                });
            }
        })
        .catch(error => console.error('Error:', error));
}

function formatDate(timestamp) {
    const date = new Date(timestamp);
    return date.toLocaleDateString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
    });
}

function getAuthorDisplayName(post) {
    const author = post.author ? post.author.trim() : '';
    if (author) {
        return `作者：${author}`;
    }
    if (post.user_id !== undefined && post.user_id !== null) {
        return `作者：${post.user_id}`;
    }
    return '作者：';
}

function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

async function deletePost(id) {
    // 显示确认对话框
    if (confirm('确定要删除这篇文章吗？删除后无法恢复。')) {
        try {
            const response = await authenticatedFetch(`/api/posts/${id}`, {
                method: 'DELETE'
            });
            const result = await response.json();
            
            if (result.success) {
                // 删除成功，重新加载文章列表
                loadPosts();
            } else {
                alert('删除失败：' + (result.message || '未知错误'));
            }
        } catch (error) {
            console.error('Error deleting post:', error);
            alert('删除失败，请稍后重试');
        }
    }
}

function editPost(id) {
    // 跳转到编辑页面
    window.location.href = `editor.html?mode=edit&id=${id}`;
}
