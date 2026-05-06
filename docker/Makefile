SHELL := /usr/bin/env bash
FZF_DEFAULT_OPTS ?='--height 50% --layout=reverse --border --exact'

.DEFAULT:
_recipe-list:
	@recipe=$$(grep -oE '^[a-z][a-zA-Z0-9-]+:' Makefile | tr -d ':' | \
	fzf --preview 'make --silent -n {} | head -n 5' --preview-window=50%:down); \
	[[ -n "$$recipe" ]] && make --silent $$recipe

APPS := docker-compose-apps.yml
DB := docker-compose-db.yml

docker-run-db:
	docker compose -f $(DB) up -d
docker-stop-db:
	docker compose -f $(DB) down -v

docker-apps-build:
	docker compose -f $(APPS) build --no-cache
docker-apps-start:
	docker compose -f $(APPS) up -d --build --force-recreate
docker-apps-stop:
	docker compose -f $(APPS) down -v
docker-apps-logs:
	docker compose -f $(APPS) logs -f

docker-full-start:
	docker compose -f $(DB) -f $(APPS) up -d --build --force-recreate --wait
docker-full-stop:
	docker compose -f $(DB) -f $(APPS) down -v
docker-full-logs:
	docker compose -f $(DB) -f $(APPS) logs -f
