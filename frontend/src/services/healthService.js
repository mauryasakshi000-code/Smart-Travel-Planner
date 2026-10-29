import { ApiError, apiRequest } from './apiClient';

/** Resolves with the backend health body, including the 503 "database disconnected" case. */
export async function fetchHealth(options) {
  try {
    return await apiRequest('/health', { timeoutMs: 5000, ...options });
  } catch (err) {
    if (err instanceof ApiError && err.status === 503 && err.data) return err.data;
    throw err;
  }
}
