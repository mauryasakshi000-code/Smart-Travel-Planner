import { apiRequest } from './apiClient';

export function register(body) {
  return apiRequest('/auth/register', { method: 'POST', body });
}

export function login(body) {
  return apiRequest('/auth/login', { method: 'POST', body });
}

export function logout() {
  return apiRequest('/auth/logout', { method: 'POST' });
}

export function fetchMe() {
  return apiRequest('/auth/me');
}
