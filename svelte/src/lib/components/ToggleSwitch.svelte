<script lang="ts">
    export let active = false;
    export let label = "";

    function toggle() {
        active = !active;
    }
</script>

<div class="setting-item">
    <div class="setting-name">{label}</div>
    <div class="toggle" class:active on:click={toggle}></div>
</div>

<style>
    .setting-item {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 0.75rem;
    }

    .setting-name {
        font-size: 0.9rem;
        opacity: 0.9;
    }

    .toggle {
        position: relative;
        width: 50px;
        height: 26px;
        background: rgba(255, 255, 255, 0.1);
        border-radius: 26px;
        cursor: pointer;
        border: 1px solid var(--glass-border);
    }

    .toggle::after {
        content: "";
        position: absolute;
        width: 22px;
        height: 22px;
        background: var(--primary);
        border-radius: 50%;
        top: 1px;
        left: 1px;
        transition: all 0.3s ease;
    }

    .toggle.active {
        background: rgba(132, 104, 255, 0.3);
    }

    .toggle.active::after {
        left: calc(100% - 23px);
        background: var(--secondary);
    }
</style>
