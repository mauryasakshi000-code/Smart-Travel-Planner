import { apiRequest } from './apiClient';

export function fetchDashboard() {
  return apiRequest('/dashboard/stats');
}
