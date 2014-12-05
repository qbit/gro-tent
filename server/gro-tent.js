var http = require('http'),
pg = require('pg'),
url = require('url'),
cstr = "postgres://%u@%h:5433/%d",
server;

cstr = cstr.replace("%u", process.env['PGUSER'] || process.env['USER']);
cstr = cstr.replace("%h", process.env['PGHOST'] || "localhost");
cstr = cstr.replace("%d", process.env['PGDATABASE'] || "%d");

if (cstr.match(/\%/)) {
    throw new Error("Please set your PG{USER,HOST,DATABASE} env vars!");
}

function dataToQs(obj) {
    var query = "insert into data (%f) values (%p);";
    var params = [];
    var fields = [];
    var values = [];
    var counter = 1;
    var result = {};

    for (v in obj) {
	params.push("$"+counter);
	fields.push(v);
	values.push(obj[v]);
	counter++;
    }

    result.query = query.replace("%p", params.join(','))
	.replace("%f", fields.join(', '));

    result.values = values;

    return result;
}

function saveData(data, res, fn) {
    data = dataToQs(data);

    pg.connect(cstr, function(err, client, done) {
	function errors(e) {
	    if (!e) {
		return false;
	    }

	    done(client);

	    console.log(e);
	    res.writeHead(500, {'Content-Type': 'text/plain'});
	    res.end(e.code);
	    return true;
	}

	if (errors(err)) {
	    return;
	}

	client.query(data.query, data.values, function(err, result) {
	    done();

	    if (errors(err)) {
		return;
		fn = null;
	    }

	    res.writeHead(200, {'Content-Type': 'http/plain'});
	    res.end();
	    if (fn) {
		fn.call(null, result);
		fn = null;
	    }
	});
    });
}

function getData(obj) {
    pg.connect(cstr, function(err, client, done) {
    });
}

server = http.createServer(function(req, res) {
    var params = url.parse(req.url, true);
    console.log(req.url);
    if (params.path.match(/^\/output/)) {
	getData(params.query, res);
    } else if (params.path.match(/^\/data/)) {
	saveData(params.query, res);
    } else {
	res.statusCode = 404;
	res.end();
    }
});

server.listen(process.env['PORT'] || 3000);
