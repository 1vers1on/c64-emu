import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vite';
import autoPreprocess from 'svelte-preprocess';

export default defineConfig({
	plugins: [sveltekit()],
	preprocess: autoPreprocess(),
});
