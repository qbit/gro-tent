google.load('visualization', '1', {
    packages: ['corechart']
});

google.setOnLoadCallback(function() {
    $.ajax({
	url: 'https://data.sparkfun.com/output/xRl4Mb5XbMcOwQA5dr2R.json',
	data: {page: 1},
	dataType: 'jsonp',
    }).done(function(results) {
	var data = new google.visualization.DataTable();
	data.addColumn('datetime', 'Time');
	data.addColumn('number', 'Humidity');
	data.addColumn('number', 'Temp (c)');
	data.addColumn('number', 'Temp (f)');
	data.addColumn('number', 'Lux');
	data.addColumn('number', 'Dew Point');

	$.each(results, function (i, row) {
	    data.addRow([
		(new Date(row.timestamp)),
		parseFloat(row.humidity),
		parseFloat(row.temp_c),
		parseFloat(row.temp_f),
		parseFloat(row.lux),
		parseFloat(row.dew_p)
	    ]);
	});

	var chart = new google.visualization.LineChart($('#chart').get(0));

	chart.draw(data, {
	    title: 'Grow Tent'
	});
    });
});
