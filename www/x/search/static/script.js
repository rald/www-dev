document.addEventListener('DOMContentLoaded', () => {
    const queryInput = document.getElementById('query');
    const searchBtn = document.getElementById('search-btn');
    const resultsList = document.getElementById('results');
    const toggleBtn = document.getElementById('toggle-submit-btn');
    const submitPanel = document.getElementById('submit-panel');
    const submitBtn = document.getElementById('submit-btn');
    const subUrl = document.getElementById('sub-url');
    const subTitle = document.getElementById('sub-title');
    const formMsg = document.getElementById('form-msg');

    // Fetch and display search results
    const performSearch = async () => {
        const q = queryInput.value.trim();
        try {
            const res = await fetch(`cgi-bin/search.py?action=search&q=${encodeURIComponent(q)}`);
            const json = await res.json();
            
            resultsList.innerHTML = '';
            if (json.data.length === 0) {
                resultsList.innerHTML = '<li>No independent sites found. Try submitting one!</li>';
                return;
            }

            json.data.forEach(site => {
                const li = document.createElement('li');
                const a = document.createElement('a');
                a.href = site.url;
                a.textContent = site.title;
                a.target = '_blank';
                
                const span = document.createElement('span');
                span.textContent = ` — ${site.url}`;
                span.style.color = '#555';
                span.style.fontSize = '0.85rem';

                li.appendChild(a);
                li.appendChild(span);
                resultsList.appendChild(li);
            });
        } catch (err) {
            console.error('Search error:', err);
        }
    };

    // Handle Submissions
    submitBtn.addEventListener('click', async () => {
        const url = subUrl.value.trim();
        const title = subTitle.value.trim();
        
        if (!url) {
            formMsg.textContent = 'URL is required.';
            return;
        }

        const formData = new URLSearchParams();
        formData.append('action', 'submit');
        formData.append('url', url);
        formData.append('title', title);

        try {
            const res = await fetch('cgi-bin/search.py', {
                method: 'POST',
                body: formData
            });
            const json = await res.json();
            formMsg.textContent = json.message;
            if (json.status === 'success') {
                subUrl.value = '';
                subTitle.value = '';
                performSearch(); // Refresh list
            }
        } catch (err) {
            formMsg.textContent = 'Error connecting to server.';
        }
    });

    // UI event listeners
    searchBtn.addEventListener('click', performSearch);
    queryInput.addEventListener('keypress', (e) => { if (e.key === 'Enter') performSearch(); });
    toggleBtn.addEventListener('click', () => submitPanel.classList.toggle('hidden'));

    // Initial load of index entries
    performSearch();
});
