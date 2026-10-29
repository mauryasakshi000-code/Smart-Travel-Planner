const API_BASE_URL = (import.meta.env.VITE_API_BASE_URL || '/api').replace(/\/$/, '');
const DEFAULT_TIMEOUT_MS = 10000;

export class ApiError extends Error {
  constructor(message, { status = 0, data = null } = {}) {
    super(message);
    this.name = 'ApiError';
    this.status = status;
    this.data = data;
  }
}

async function parseBody(response) {
  const text = await response.text();
  if (!text) return null;
  try {
    return JSON.parse(text);
  } catch {
    return { message: text };
  }
}

/**
 * JSON fetch wrapper used by every service module.
 * Throws ApiError with `status` 0 for network failures/timeouts, otherwise the HTTP status.
 */
export async function apiRequest(
  path,
  { method = 'GET', body, headers, signal, timeoutMs = DEFAULT_TIMEOUT_MS } = {},
) {
  const controller = new AbortController();
  let timedOut = false;
  const timer = setTimeout(() => {
    timedOut = true;
    controller.abort();
  }, timeoutMs);
  const forwardAbort = () => controller.abort();
  signal?.addEventListener('abort', forwardAbort, { once: true });

  try {
    const response = await fetch(`${API_BASE_URL}${path}`, {
      method,
      credentials: 'include',
      headers: {
        Accept: 'application/json',
        ...(body !== undefined && { 'Content-Type': 'application/json' }),
        ...headers,
      },
      body: body !== undefined ? JSON.stringify(body) : undefined,
      signal: controller.signal,
    });

    const data = await parseBody(response);
    if (!response.ok) {
      throw new ApiError(data?.error || `Request failed with status ${response.status}`, {
        status: response.status,
        data,
      });
    }
    return data;
  } catch (err) {
    if (err instanceof ApiError || signal?.aborted) throw err;
    if (timedOut) throw new ApiError('The server took too long to respond. Please try again.');
    throw new ApiError('Unable to reach the server. Check that the backend is running.');
  } finally {
    clearTimeout(timer);
    signal?.removeEventListener('abort', forwardAbort);
  }
}
