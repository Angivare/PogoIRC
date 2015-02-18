window.onload = function() {
	window.scrollTo(0,document.body.scrollHeight);
};

function docHeight() {
	var body = document.body.scrollHeight;
	return body;
}

function scrollHeight() {
	var top = document.body.scrollTop;
	var viewport = window.innerHeight;
	return top + viewport;
}

function imageLoad(ev, img) {
	img.style.display = "none";
	var scrollBefore = scrollHeight();
	var heightBefore = docHeight();
	img.style.display = "initial";
	
	scroll(scrollBefore, heightBefore, docHeight());
}

function scroll(scrollBefore, heightBefore, heightAfter) {
	if(scrollBefore >= heightBefore)
	{	//scroll to bot
		window.scrollTo(0, heightAfter);
	}
}

var n = 0;
function addPost(post, nick) {
	var scrollBefore = scrollHeight();
	var heightBefore = docHeight();

	var elemPost = document.createElement("div");
	elemPost.className = "app-post"+(n%2);
	elemPost.innerHTML = post;
	var images = elemPost.getElementsByTagName("img");
	for(var i = 0; i < images.length; i++)
		images[i].onload = function(ev) { imageLoad(ev, this); };
	document.getElementById("app-contents").appendChild(elemPost);
	
	scroll(scrollBefore, heightBefore, docHeight());

	n++;
	onNewPost(elemPost);
}

function clearPosts(max) {
	var posts = document.getElementsByClassName("app-post");
	if(posts.length > max && posts.length > 2) //Minimum post limit
	{ //Clear second post
		document.getElementById("app-contents").removeChild(posts[1]);
		//To get this function working, app-post classes MUST be children of
		//app-contents div. Do not ever put app-contents in another parent

		clearPosts(max);
	}
}
