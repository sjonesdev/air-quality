.PHONY: start-db-image init-db docker-psql overwrite-schema migrate-db inspect-schema

start-db-image:
	docker run --rm -d \
		--name air-quality-db -p 3306:5432 \
		-e POSTGRES_PASSWORD=pass postgres:16

migrate-db: # see if can rely on schema.sql
	atlas schema apply \
		--url "postgresql://postgres:pass@localhost:3306/postgres?sslmode=disable" \
		--to "file://db/schema.sql" \
		--dev-url "docker://postgres/16"

init-db: start-db-image migrate-db
	echo "Database initialized in docker container and schema applied"

overwrite-schema-from-db:
	atlas schema inspect \
		-u "postgresql://postgres:pass@localhost:3306/postgres?sslmode=disable" \
		--format '{{ sql . }}' > schema.sql

inspect-local-schema:
	atlas schema inspect \
		-u "postgresql://postgres:pass@localhost:3306/postgres?sslmode=disable" \
		--web
