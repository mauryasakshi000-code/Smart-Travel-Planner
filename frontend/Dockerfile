# syntax=docker/dockerfile:1

# ---------- Development server (Vite with hot reload) ----------
FROM node:24-alpine AS dev

WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci

COPY . .
EXPOSE 5173
CMD ["npm", "run", "dev"]
