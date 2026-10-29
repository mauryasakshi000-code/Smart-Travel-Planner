
import { defineConfig, loadEnv } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig(({ mode }) => {
  const env = loadEnv(mode, process.cwd(), '');

  return {
    plugins: [react()],
    server: {
      host: true,
      port: 5173,
      strictPort: true,
      watch:
        env.WATCH_POLLING === 'true'
          ? { usePolling: true, interval: 300 }
          : undefined,
      proxy: {
        '/api/health': {
          target: env.API_PROXY_TARGET || 'http://localhost:8080',
          changeOrigin: true,
          rewrite: () => '/health',
        },
        '/api': {
          target: env.API_PROXY_TARGET || 'http://localhost:8080',
          changeOrigin: true,
        },
      },
    },
  };
});
