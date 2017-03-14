#! /usr/bin/env ruby

require 'json'
require 'date'

def check_config(cfg)
  abort "no mission name" if not cfg.key? "name"
  abort "no payloads specified" if not cfg.key? "payloads"
  
  payload_ids = []
  
  cfg["payloads"].each do |payload|
    abort "no payload address" if not payload.key? "address"
    abort "no payload name"  if not payload.key? "name"
    abort "no payload priority"  if not payload.key? "priority"
    abort "duplicate payload addresses"  if payload_ids.include? payload["address"]
    abort "invalid payload address" if payload["address"] < 10 or payload["address"] > 127
    payload_ids.push(payload["address"])
  end
end

def write_config(f, config)
  
  f.puts "// FCORE Configuration File - #{config["name"]}"
  f.puts "// Generated on #{Date.today} by fcore_config.rb"
  f.puts "\#pragma once"
  f.puts ""
  f.puts "\#define FCORE_MISSION_NAME    \"#{config["name"]}\""
  f.puts "\#define FCORE_CONFIG_GENDATE  \"#{Date.today}\""
  f.puts "\#define FCORE_PAYLOAD_COUNT   (#{config["payloads"].count})"
  f.puts ""
  f.puts "static const FCPayload fcore_payloads[] = {"
  config["payloads"].each do |payload|
    f.puts "    {#{payload["address"]}, \"#{payload["name"]}\", #{payload["priority"]}}"
  end
  f.puts "};"
end

if ARGV.count != 2 then
  puts "usage: fcore_config infile outfile"
  puts "\tinfile:\tFCORE json configuration file"
  puts "\toutfile:\toutput C header file"
  abort "aborting..."
end

data = File.read(ARGV[0])
config = JSON.parse(data)
out = File.open(ARGV[1], "w")

check_config config
write_config out, config
out.close
