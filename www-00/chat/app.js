// ==========================================================================
// 1. DATA CORE LAYER (MODEL)
// ==========================================================================
class ChatModel {
    constructor() {
        this.rawMessages = [];
        this.groupedMessages = {}; // Struct: { "YYYY-MM-DD": [ {time: "HH:MM:SS", text: "..."}, ... ] }
        this.availableDates = [];
        this.onChanges = [];
    }

    bindOnChange(callback) {
        this.onChanges.push(callback);
    }

    async fetchMessages(forceScroll = false) {
        try {
            const res = await fetch('/api/chat');
            const text = await res.text();
            this.rawMessages = text.split('\n').filter(line => line.trim() !== '');
            
            this.processGroupings();
            
            this.onChanges.forEach(cb => cb(this.groupedMessages, this.availableDates, forceScroll));
        } catch (e) {
            console.error("Error fetching chat data:", e);
        }
    }

    processGroupings() {
        this.groupedMessages = {};
        const datesSet = new Set();

        this.rawMessages.forEach(msg => {
            // Updated matching expression for: [YYYY-MM-DD HH:MM:SS] Message
            const match = msg.match(/^\[(\d{4}-\d{2}-\d{2})\s(\d{2}:\d{2}:\d{2})\]\s(.*)/);
            
            if (match) {
                const dateKey = match[1];
                const timeStr = match[2];
                const content = match[3];

                if (!this.groupedMessages[dateKey]) {
                    this.groupedMessages[dateKey] = [];
                }
                this.groupedMessages[dateKey].push({ time: timeStr, text: content });
                datesSet.add(dateKey);
            } else {
                // Retrocompatible fallback bucket for messages without full date formatting
                const legacyKey = "Legacy Logs";
                if (!this.groupedMessages[legacyKey]) {
                    this.groupedMessages[legacyKey] = [];
                }
                this.groupedMessages[legacyKey].push({ time: "--:--:--", text: msg });
                datesSet.add(legacyKey);
            }
        });

        this.availableDates = Array.from(datesSet).sort();
    }

    async sendMessage(user, message) {
        if (!message.trim()) return;
        const payload = `${user}: ${message}`;
        
        try {
            await fetch('/api/chat', {
                method: 'POST',
                body: payload
            });
            this.fetchMessages(true);
        } catch (e) {
            console.error("Error dispatching payload packet:", e);
        }
    }
}

// ==========================================================================
// 2. DOM MANAGEMENT LAYER (VIEW)
// ==========================================================================
class ChatView {
    constructor() {
        this.chatbox = document.getElementById('chatbox');
        this.username = document.getElementById('username');
        this.msgInput = document.getElementById('msgInput');
        this.sendBtn = document.getElementById('sendBtn');
        this.themeToggle = document.getElementById('themeToggle');
        this.dateFilter = document.getElementById('dateFilter');
    }

    setThemeAttr(theme) {
        document.documentElement.setAttribute('data-theme', theme);
        this.themeToggle.textContent = theme === 'light' ? '🌙' : '☀️';
    }

    updateDropdownOptions(dates, targetActiveDay) {
        // Only re-render options array if lengths differ to prevent select menu focus loss
        if (this.dateFilter.options.length === dates.length) return;

        this.dateFilter.textContent = '';
        dates.forEach(date => {
            const opt = document.createElement('option');
            opt.value = date;
            opt.textContent = date;
            if (date === targetActiveDay) opt.selected = true;
            this.dateFilter.appendChild(opt);
        });
    }

    renderDayView(dayMessages, forceScroll = false) {
        const threshold = 50;
        const isAtBottom = (this.chatbox.scrollHeight - this.chatbox.clientHeight - this.chatbox.scrollTop) <= threshold;

        this.chatbox.textContent = ''; 

        if (!dayMessages || dayMessages.length === 0) {
            const emptyNotice = document.createElement('div');
            emptyNotice.style.boxShadow = 'none';
            emptyNotice.style.justifyContent = 'center';
            emptyNotice.style.color = 'var(--text-muted)';
            emptyNotice.textContent = "No log records found for this day profile.";
            this.chatbox.appendChild(emptyNotice);
            return;
        }

        dayMessages.forEach(msg => {
            const bubble = document.createElement('div');
            
            const timeSpan = document.createElement('span');
            timeSpan.className = 'chat-timestamp';
            timeSpan.textContent = `[${msg.time}]`;
            
            const textSpan = document.createElement('span');
            textSpan.className = 'chat-content';
            textSpan.textContent = msg.text;
            
            bubble.appendChild(timeSpan);
            bubble.appendChild(textSpan);
            this.chatbox.appendChild(bubble);
        });

        if (isAtBottom || forceScroll) {
            this.chatbox.scrollTop = this.chatbox.scrollHeight;
        }
    }

    get selectedViewDay() {
        return this.dateFilter.value;
    }

    get inputData() {
        return {
            user: this.username.value.trim().substring(0, 32) || "Anonymous",
            message: this.msgInput.value
        };
    }

    clearInput() {
        this.msgInput.value = '';
    }
}

// ==========================================================================
// 3. EVENT ROUTING & ORCHESTRATION LAYER (CONTROLLER)
// ==========================================================================
class ChatController {
    constructor(model, view) {
        this.model = model;
        this.view = view;
        this.userSelectedDay = null;

        const savedTheme = localStorage.getItem('chat-theme') || 'dark';
        this.currentTheme = savedTheme;
        this.view.setThemeAttr(this.currentTheme);

        // Core Layout Event Listeners Bindings
        this.model.bindOnChange((grouped, dates, forceScroll) => this.handleModelUpdate(grouped, dates, forceScroll));
        
        this.view.sendBtn.addEventListener('click', () => this.handleSend());
        this.view.msgInput.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') this.handleSend();
        });
        
        this.view.themeToggle.addEventListener('click', () => this.toggleTheme());
        this.view.dateFilter.addEventListener('change', () => {
            this.userSelectedDay = this.view.selectedViewDay;
            this.model.fetchMessages(true); // Force scroll reset to bottom when transitioning days
        });

        this.model.fetchMessages(true);
        setInterval(() => this.model.fetchMessages(false), 2000);
    }

    getSystemTodayString() {
        const d = new Date();
        const yyyy = d.getFullYear();
        const mm = String(d.getMonth() + 1).padStart(2, '0');
        const dd = String(d.getDate()).padStart(2, '0');
        return `${yyyy}-${mm}-${dd}`;
    }

    handleModelUpdate(grouped, dates, forceScroll) {
        const todayStr = this.getSystemTodayString();

        // Fallback checks: default view mapping to the present day
        if (!this.userSelectedDay) {
            if (dates.includes(todayStr)) {
                this.userSelectedDay = todayStr;
            } else if (dates.length > 0) {
                // If there are logs but none match today, default display to the most recent chat activity day
                this.userSelectedDay = dates[dates.length - 1];
            } else {
                this.userSelectedDay = todayStr;
            }
        }

        // Dynamically maintain dropdown options array list
        const selectOptionsList = dates.includes(todayStr) ? dates : [...dates, todayStr].sort();
        this.view.updateDropdownOptions(selectOptionsList, this.userSelectedDay);

        const currentDisplayData = grouped[this.userSelectedDay] || [];
        this.view.renderDayView(currentDisplayData, forceScroll);
    }

    toggleTheme() {
        this.currentTheme = this.currentTheme === 'dark' ? 'light' : 'dark';
        localStorage.setItem('chat-theme', this.currentTheme);
        this.view.setThemeAttr(this.currentTheme);
    }

    handleSend() {
        const { user, message } = this.view.inputData;
        if (message.trim()) {
            // Auto-switch view viewport target instantly back to today if posting from an old archive index day
            this.userSelectedDay = this.getSystemTodayString();
            this.model.sendMessage(user, message);
            this.view.clearInput();
        }
    }
}

const app = new ChatController(new ChatModel(), new ChatView());
