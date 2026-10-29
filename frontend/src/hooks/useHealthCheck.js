import { useCallback, useEffect, useState } from 'react';

import { fetchHealth } from '../services/healthService';

export function useHealthCheck(pollIntervalMs = 15000) {
  const [health, setHealth] = useState(null);
  const [error, setError] = useState(null);
  const [loading, setLoading] = useState(true);
  const [checkedAt, setCheckedAt] = useState(null);

  const check = useCallback(async (signal) => {
    setLoading(true);
    try {
      const data = await fetchHealth({ signal });
      if (signal?.aborted) return;
      setHealth(data);
      setError(null);
    } catch (err) {
      if (signal?.aborted) return;
      setHealth(null);
      setError(err.message);
    } finally {
      if (!signal?.aborted) {
        setLoading(false);
        setCheckedAt(new Date());
      }
    }
  }, []);

  useEffect(() => {
    const controller = new AbortController();
    check(controller.signal);
    const timer = setInterval(() => check(controller.signal), pollIntervalMs);
    return () => {
      controller.abort();
      clearInterval(timer);
    };
  }, [check, pollIntervalMs]);

  return { health, error, loading, checkedAt, refresh: () => check() };
}
