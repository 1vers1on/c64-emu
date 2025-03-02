<script lang="ts">
    import "$lib/styles/variables.css";
    interface Props {
        children?: import('svelte').Snippet;
    }

    let { children }: Props = $props();
</script>

{@render children?.()}
