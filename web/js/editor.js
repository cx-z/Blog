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
        const response = await fetch('/api/posts', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                title: title,
                content: content
            })
        });
        
        const result = await response.json();
        
        if (result.success) {
            // 显示成功消息
            document.getElementById('success-message').style.display = 'block';
            document.getElementById('error-message').style.display = 'none';
            
            // 清空表单
            document.getElementById('post-form').reset();
            document.getElementById('post-form').classList.remove('was-validated');
            
            // 3 秒后跳转回首页
            setTimeout(() => {
                window.location.href = 'index.html';
            }, 2000);
        } else {
            showError(result.message || '发布失败，请重试');
        }
    } catch (error) {
        console.error('Error:', error);
        showError('网络错误，请检查服务器是否正在运行');
    }
});

function showError(message) {
    document.getElementById('error-message').style.display = 'block';
    document.getElementById('error-text').textContent = message;
    document.getElementById('success-message').style.display = 'none';
}
