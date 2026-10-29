import { apiRequest } from './apiClient';

export function fetchMyStories() {
  return apiRequest('/stories');
}

export function fetchPublicStories() {
  return apiRequest('/stories/public');
}

export function fetchStory(id) {
  return apiRequest(`/stories/${id}`);
}

export function createStory(body) {
  return apiRequest('/stories', { method: 'POST', body });
}

export function updateStory(id, body) {
  return apiRequest(`/stories/${id}`, { method: 'PUT', body });
}

export function deleteStory(id) {
  return apiRequest(`/stories/${id}`, { method: 'DELETE' });
}

export function publishStory(id) {
  return apiRequest(`/stories/${id}/publish`, { method: 'POST' });
}

export function fetchPlaces() {
  return apiRequest('/places');
}

export function addPlace(body) {
  return apiRequest('/places', { method: 'POST', body });
}

export function deletePlace(id) {
  return apiRequest(`/places/${id}`, { method: 'DELETE' });
}
