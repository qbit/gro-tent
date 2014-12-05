var http = require('http'),
pg = require('pg'),
url = require('url'),
cstr = "postgres://%u@%h/%d",
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
    console.log(data);


    pg.connect(cstr, function(err, client, done) {
	function errors(err) {
	    if (!err) {
		return false;
	    }

	    done(client);

	    res.writeHead(500, {'Content-Type': 'text/plain'});
	    res.end(err.code);
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

	    console.log(result.rows[0].number);
	    fn.call(null, {str: "saved"});
	    fn = null;
	});
    });
}

server = http.createServer(function(req, res) {
    var params = url.parse(req.url, true);

    if (params.path.match(/^\/data/)) {
	saveData(params.query, res, function(status) {
	});
    } else {
	res.statusCode = 404;
	res.end();
    }
});

server.listen(process.env['PORT']);
