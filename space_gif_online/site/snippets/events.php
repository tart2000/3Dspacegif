  <div class="col-md-12">
    <h2>Events</h2>

  </div>
  <div class="row">

    <?php foreach ($page('gifs')->images() as $image) : ?>
      <div class="col-md-3">
        <a href="<?php echo $image->url() ?>">
          <img src="<?php echo $image->url() ?>" alt="<?php echo $project->title()->html() ?>" class="img-responsive">
        </a>
      </div>
    <?php endforeach ?>

  </div>

