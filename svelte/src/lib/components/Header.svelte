<script lang="ts">
</script>

<header>
    <div class="logo">
        <div class="logo-icon">
            <i class="fas fa-terminal"></i>
        </div>
        <h1>Commodore 64 Emulator test</h1>
    </div>
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
        background-clip: text;
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
</style>
