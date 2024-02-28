/*
 * Copyright (C) Samuel Jones - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Samuel Jones <spjones329@gmail.com>, February 2024
 */

package main

import (
	"SamJones329/airquality/db"
	"context"
	"fmt"
	"log"
	"net"
	"net/http"
	"strings"
	"time"

	"html/template"

	"github.com/gorilla/websocket"
	"github.com/jackc/pgx/v5"
)

var upgrader = websocket.Upgrader{}

func handleSensorConn(conn net.Conn, dbconn *db.Queries, ctx context.Context) {
	log.Printf("Accepted connection from %v", conn.RemoteAddr())
	defer conn.Close()
	// message should be 15 characters, 3 hexademical numbers in the order co2, temp, humidity in the form "XXXX XXXX XXXX\n"
	buf := make([]byte, 9)
	createParams := db.CreateAirStatParams{}
	for {
		n, err := conn.Read(buf)
		if err != nil {
			log.Printf("Read error, closing connection: %v", err)
			return
		}
		if n != 9 || buf[2] != ' ' || buf[5] != ' ' || buf[8] != '\n' {
			log.Printf("Message formatted incorrectly, ignoring")
			continue
		}

		createParams.Co2Ppm = int32(buf[0]) + (int32(buf[1]) << 8)
		createParams.TempTick = int32(buf[3]) + (int32(buf[4]) << 8)
		createParams.HumidityTick = int32(buf[6]) + (int32(buf[7]) << 8)

		_, err = dbconn.CreateAirStat(ctx, createParams)
		if err != nil {
			log.Printf("Failed to insert into database: %v", err)
			continue
		}
	}
}

type ClientAirStats struct {
	// comma separated, no brackets
	Dates  string
	Co2s   []int32
	Temps  []int32
	Humids []int32
}

func main() {
	ln, err := net.Listen("tcp", ":8080")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	defer ln.Close()
	fmt.Println("Listening on port 8080")

	ctx := context.Background()
	dbconn, err := pgx.Connect(ctx, DB_URL)
	if err != nil {
		log.Fatalf("failed to connect to database: %v", err)
	}
	defer dbconn.Close(ctx)

	queries := db.New(dbconn)

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		t, err := template.ParseFiles("templ/index.html")
		if err != nil {
			log.Printf("failed to parse template: %v", err)
		}

		stats, err := queries.GetAirStatsPerFiveMinutes(ctx)
		if err != nil {
			log.Printf("failed to get stats: %v", err)
		}
		dates := make([]string, len(stats))
		co2s := make([]int32, len(stats))
		temps := make([]int32, len(stats))
		humids := make([]int32, len(stats))
		for i, stat := range stats {
			dates[i] = stat.FiveMinuteIntervalStart.Time.String()
			co2s[i] = stat.Co2Ppm
			temps[i] = stat.TempTick
			humids[i] = stat.HumidityTick
		}
		clientStats := ClientAirStats{Dates: strings.Join(dates, ","), Co2s: co2s, Temps: temps, Humids: humids}

		// err = t.ExecuteTemplate(w, "T", clientStats)
		err = t.Execute(w, clientStats)
		if err != nil {
			log.Printf("failed to execute template: %v", err)
		}
	})

	http.HandleFunc("/datastream", func(w http.ResponseWriter, r *http.Request) {
		c, err := upgrader.Upgrade(w, r, nil)
		if err != nil {
			log.Print("upgrade:", err)
			return
		}
		defer c.Close()
		for {
			time.Sleep(5 * time.Second)
			stat, err := queries.GetLatestAirStat(ctx)
			if err != nil {
				log.Printf("failed to get latest air stat: %v", err)
				continue
			}
			err = c.WriteMessage(websocket.TextMessage, []byte(fmt.Sprintf("%s,%d,%d,%d", stat.CreatedAt.Time.String(), stat.Co2Ppm, stat.TempTick, stat.HumidityTick)))
			if err != nil {
				log.Println("write:", err)
				break
			}
		}
	})
	go http.ListenAndServe(":80", nil)

	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Printf("failed to accept: %v", err)
			continue
		}
		go handleSensorConn(conn, queries, ctx)
	}
}
