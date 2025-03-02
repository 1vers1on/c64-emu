<script lang="ts">
    interface Props {
        label?: string;
        min?: number;
        max?: number;
        value?: number;
        unit?: string;
    }

    let {
        label = "",
        min = 0,
        max = 100,
        value = $bindable(50),
        unit = "%"
    }: Props = $props();

    let displayValue = $derived(`${value}${unit}`);
</script>

<div class="slider-control">
    <div class="slider-label">
        <span>{label}</span>
        <span>{displayValue}</span>
    </div>
    <input type="range" bind:value {min} {max} class="slider" />
</div>

<style>
    .slider-control {
        display: flex;
        flex-direction: column;
        gap: 0.5rem;
        width: 100%;
    }

    .slider-label {
        display: flex;
        justify-content: space-between;
        font-size: 0.85rem;
        opacity: 0.8;
    }

    .slider {
        appearance: none;
        -webkit-appearance: none;
        width: 100%;
        height: 6px;
        background: rgba(255, 255, 255, 0.1);
        border-radius: 5px;
        outline: none;
    }

    .slider::-webkit-slider-thumb {
        -webkit-appearance: none;
        appearance: none;
        width: 16px;
        height: 16px;
        border-radius: 50%;
        background: var(--secondary);
        cursor: pointer;
    }
</style>
