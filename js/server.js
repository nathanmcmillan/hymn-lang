#!/usr/bin/env node

/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

const http = require('http')
const fs = require('fs')
const path = require('path')

const port = 3000
const directory = './'

const extensions = {
  '.html': 'text/html',
  '.css': 'text/css',
  '.js': 'text/javascript',
  '.json': 'application/json',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.svg': 'image/svg+xml',
  '.ico': 'image/x-icon',
  '.wav': 'audio/wav',
}

const server = http.createServer(function (request, response) {
  console.log('request', request.url)
  let file = request.url.split('?')[0]
  if (file === '/') file = '/index.html'
  else if (file.indexOf('.') === -1) file += '.html'
  file = directory + file
  const extension = path.extname(file)
  const mime = extensions[extension] || 'text/plain'
  fs.readFile(file, function (error, content) {
    if (error) {
      response.writeHead(404, { 'Content-Type': 'text/html' })
      fs.readFile(directory + '404.html', function (error, content) {
        if (error) {
          response.end('404', 'utf-8')
        } else {
          response.end(content, 'utf-8')
        }
      })
    } else {
      response.writeHead(200, { 'Content-Type': mime })
      response.end(content, 'utf-8')
    }
  })
})

server.listen(port)

console.log('serving on port', port)
