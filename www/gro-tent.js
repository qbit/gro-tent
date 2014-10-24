google.load('visualization', '1', {
    packages: ['corechart']
});

function makeChart(name, data, id, row_defs) {
    var tmp_data = new google.visualization.DataTable();

    var i, l;

    for (i = 0, l = row_defs.length; i < l; i++) {
	tmp_data.addColumn(row_defs[i].type, row_defs[i].name);
    }

    tmp_data.addRows(data.length || 0);

    for (i = 0, l = data.length; i < l; i++) {
	var row = data[i];
	var j, k;

	for (j = 0, k = row_defs.length; j < k; j++) {
	    var value = row[row_defs[j].value];
	    switch (row_defs[j].type) {
		case "datetime":
		    value = new Date(value);
		    break;
		case "number":
		    value = parseFloat(value);
 		    break;
	    }

	    tmp_data.setCell(i, j, value);
	}
    }

    var chart = new google.visualization.LineChart($('#'+id).get(0));

    chart.draw(tmp_data, {
	title: name
    });
}

google.setOnLoadCallback(function() {
    $.ajax({
	url: 'https://data.sparkfun.com/output/xRl4Mb5XbMcOwQA5dr2R.json',
	data: {page: 1},
	dataType: 'jsonp',
    }).done(function(results) {
	makeChart('Temperature', results, 'temp', [
	    {type: 'datetime', name: 'Time', value: 'timestamp'},
	    {type: 'number', name: 'Temp (c)', value: 'temp_c'},
	    {type: 'number', name: 'Temp (f)', value: 'temp_f'}
	]);
	makeChart('Humidity', results, 'humid', [
	    {type: 'datetime', name: 'Time', value: 'timestamp'},
	    {type: 'number', name: 'Humidity', value: 'humidity'}
	]);

	makeChart('Lux', results, 'lux', [
	    {type: 'datetime', name: 'Time', value: 'timestamp'},
	    {type: 'number', name: 'Lux', value: 'lux'}
	]);
	
	makeChart('Temperature with Humidity', results, 'temp_h', [
	    {type: 'datetime', name: 'Time', value: 'timestamp'},
	    {type: 'number', name: 'Humidity', value: 'humidity'},
	    {type: 'number', name: 'Dew point', value: 'dew_p'},
	    {type: 'number', name: 'Temp (c)', value: 'temp_c'}
	]);

	makeChart('All', results, 'all', [
    	    {type: 'datetime', name: 'Time', value: 'timestamp'},
	    {type: 'number', name: 'Humidity', value: 'humidity'},
	    {type: 'number', name: 'Lux', value: 'lux'},
	    {type: 'number', name: 'Dew point', value: 'dew_p'},
	    {type: 'number', name: 'Temp (c)', value: 'temp_c'}
   	]); 
    });
});
