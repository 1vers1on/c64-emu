<script>
    let isDarkMode = true;
    // import { writable } from 'svelte/store';

    // export const theme = writable(isDarkMode);


    function toggleTheme() {
        isDarkMode = !isDarkMode;
        // theme.toggle();
    }
</script>

<header>
    <div class="logo">
        <div class="logo-icon">
            <i class="fas fa-terminal"></i>
        </div>
        <h1>Commodore 64 Emulator test</h1>
    </div>
    <!-- <button class="toggle-theme" on:click={toggleTheme}>
        <i class={isDarkMode ? "fas fa-moon" : "fas fa-sun"}></i>
    </button> -->
</header>

<style>
    header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 2rem;
    }

    .logo {
        display: flex;
        align-items: center;
        gap: 1rem;
    }

    .logo h1 {
        font-size: 1.8rem;
        font-weight: 700;
        background: linear-gradient(to right, var(--primary), var(--secondary));
        -webkit-background-clip: text;
        -webkit-text-fill-color: transparent;
        text-shadow: 0 0 10px rgba(132, 104, 255, 0.5);
    }

    .logo-icon {
        background: linear-gradient(135deg, var(--primary), var(--secondary));
        width: 40px;
        height: 40px;
        border-radius: 10px;
        display: flex;
        align-items: center;
        justify-content: center;
        box-shadow: 0 0 15px rgba(132, 104, 255, 0.5);
    }

    .toggle-theme {
        background: var(--glass-bg);
        border: 1px solid var(--glass-border);
        border-radius: 50px;
        padding: 0.5rem 1rem;
        /* backdrop-filter: blur(10px);
        -webkit-backdrop-filter: blur(10px); */
        cursor: pointer;
        transition: all 0.3s ease;
    }

    .toggle-theme:hover {
        background: rgba(40, 40, 55, 0.7);
    }
</style>
